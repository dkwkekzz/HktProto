// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktSimulationSubsystem.h"
#include "HktFlowBuilder.h"
#include "HktIntentInterface.h"
#include "HktServiceSubsystem.h"
#include "Flow/FlowDefinitionRegistry.h"
#include "Flow/IFlowDefinition.h"
#include "VM/HktVMPool.h"
#include "VM/HktBytecodePool.h"
#include "Core/HktSimulationStats.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY(LogHktSimulation);

void UHktSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// HktServiceSubsystem 의존성 초기화 및 Provider 등록
	if (UHktServiceSubsystem* Service = Collection.InitializeDependency<UHktServiceSubsystem>())
	{
		Service->RegisterPlayerAttributeProvider(this);
	}

	// Initialize VM pool
	VMPool = MakeUnique<FHktVMPool>();
	VMPool->SetMaxPoolSize(100);
	VMPool->Prewarm(10); // Start with 10 VMs

	// Prewarm bytecode pool
	FBytecodePool::Prewarm(20);

	UE_LOG(LogHktSimulation, Log, TEXT("HktSimulationSubsystem Initialized with object pooling"));
}

void UHktSimulationSubsystem::Deinitialize()
{
	// Unregister Provider
	if (UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld()))
	{
		Service->UnregisterPlayerAttributeProvider(this);
	}

	// Release all active VMs back to pool
	for (FHktFlowVM* VM : ActiveVMs)
	{
		if (VMPool.IsValid())
		{
			VMPool->Release(VM);
		}
	}
	ActiveVMs.Empty();
	
	VMPool.Reset();
	FBytecodePool::Clear();
	
	ExternalToInternalMap.Empty();
	LastProcessedFrame = 0;

	UE_LOG(LogHktSimulation, Log, TEXT("HktSimulationSubsystem Deinitialized"));

	Super::Deinitialize();
}

void UHktSimulationSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. IntentEvent 수집 및 처리 (Sliding Window 방식)
	ProcessIntentEvents(DeltaTime);

	// 2. 활성 VM들 틱
	TickActiveVMs(DeltaTime);

	// 3. 완료된 VM 정리
	CleanupFinishedVMs();
	
	// Note: Player 속성 동기화는 HktIntent에서 Provider를 폴링하여 처리
}

TStatId UHktSimulationSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UHktSimulationSubsystem, STATGROUP_Tickables);
}

bool UHktSimulationSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

UHktSimulationSubsystem* UHktSimulationSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}

	return World->GetSubsystem<UHktSimulationSubsystem>();
}

FUnitHandle UHktSimulationSubsystem::GetOrCreateInternalHandle(int32 ExternalUnitId, FVector Location, FRotator Rotation)
{
	SCOPE_CYCLE_COUNTER(STAT_HandleMapping);

	// 이미 매핑이 있는지 확인
	if (FUnitHandle* Found = ExternalToInternalMap.Find(ExternalUnitId))
	{
		if (EntityManager.IsUnitValid(*Found))
		{
			return *Found;
		}
		// 무효한 핸들이면 제거하고 새로 생성
		ExternalToInternalMap.Remove(ExternalUnitId);
	}

	// 새 유닛 할당
	FPlayerHandle OwnerPlayer; // TODO: 외부에서 플레이어 정보도 전달받아야 함
	FUnitHandle NewHandle = EntityManager.AllocUnit(OwnerPlayer, Location, Rotation);

	// [Optimized] 매핑 등록 - 양방향 맵 대신 단방향 + EntityDB 사용
	ExternalToInternalMap.Add(ExternalUnitId, NewHandle);
	EntityManager.Entities.ExternalIds[NewHandle.Index] = ExternalUnitId;

	// Update entity stats
	int32 ActiveCount = 0;
	for (bool bActive : EntityManager.Entities.IsActive)
	{
		if (bActive) ActiveCount++;
	}
	SET_DWORD_STAT(STAT_TotalEntities, EntityManager.Entities.Attributes.Num());
	SET_DWORD_STAT(STAT_ActiveEntities, ActiveCount);

	return NewHandle;
}

int32 UHktSimulationSubsystem::GetExternalUnitId(FUnitHandle InternalHandle) const
{
	// [Optimized] O(1) 배열 접근으로 역방향 조회
	if (EntityManager.IsUnitValid(InternalHandle))
	{
		return EntityManager.Entities.ExternalIds[InternalHandle.Index];
	}
	return INDEX_NONE;
}

void UHktSimulationSubsystem::ExecuteIntentEvent(const FHktIntentEvent& Event)
{
	// Subject의 내부 핸들 획득 (없으면 생성)
	FUnitHandle SubjectHandle = GetOrCreateInternalHandle(Event.Subject.Value, Event.Location);

	// [Optimized] Acquire VM from pool instead of creating new
	FHktFlowVM* NewVM = VMPool->Acquire(GetWorld(), &EntityManager, SubjectHandle);

	// 이벤트에 따른 바이트코드 빌드
	BuildBytecodeForEvent(*NewVM, Event);

	// 바이트코드가 비어있지 않으면 실행 큐에 추가
	if (NewVM->Bytecode.Num() > 0)
	{
		ActiveVMs.Add(NewVM);
		UE_LOG(LogHktSimulation, Verbose, TEXT("Queued VM for IntentEvent: %s (EventId: %d)"), 
			*Event.EventTag.ToString(), Event.EventId);
	}
	else
	{
		// No bytecode generated, release VM back to pool immediately
		VMPool->Release(NewVM);
	}
}

void UHktSimulationSubsystem::ProcessIntentEvents(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_ProcessIntentEvents);

	// ServiceSubsystem에서 IntentEventProvider 획득
	UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld());
	if (!Service)
	{
		return;
	}

	TScriptInterface<IHktIntentEventProvider> Provider = Service->GetIntentEventProvider();
	if (!Provider)
	{
		return;
	}

	// [Sliding Window] 마지막 처리 프레임 이후의 새 이벤트만 조회
	TArray<FHktIntentEvent> NewEvents;
	if (!Provider->FetchNewEvents(LastProcessedFrame, NewEvents))
	{
		return; // 새 이벤트 없음
	}

	// 이벤트 처리
	for (const FHktIntentEvent& Event : NewEvents)
	{
		// 이벤트 처리 시도
		ExecuteIntentEvent(Event);

		// [중요] 처리 성공 시 커서 업데이트
		// 만약 처리 도중 실패하거나 yield 해야 한다면 커서를 업데이트하지 않음으로써
		// 다음 프레임에 다시 시도할 수 있는 기회를 가짐 (재진입성 확보)
		LastProcessedFrame = FMath::Max(LastProcessedFrame, Event.FrameNumber);
		
		INC_DWORD_STAT(STAT_EventsProcessed);
	}

	if (NewEvents.Num() > 0)
	{
		UE_LOG(LogHktSimulation, Verbose, TEXT("Processed %d events, Cursor updated to %d"), 
			NewEvents.Num(), LastProcessedFrame);
	}
}

void UHktSimulationSubsystem::TickActiveVMs(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_TickVMs);

	for (FHktFlowVM* VM : ActiveVMs)
	{
		if (VM)
		{
			VM->Tick(DeltaTime);
		}
	}

	// Update stats
	SET_DWORD_STAT(STAT_ActiveVMs, ActiveVMs.Num());
}

void UHktSimulationSubsystem::CleanupFinishedVMs()
{
	SCOPE_CYCLE_COUNTER(STAT_CleanupFinishedVMs);

	// [Optimized] Release finished VMs back to pool
	for (int32 i = ActiveVMs.Num() - 1; i >= 0; --i)
	{
		FHktFlowVM* VM = ActiveVMs[i];
		
		if (!VM || VM->Regs.ProgramCounter >= VM->Bytecode.Num())
		{
			// VM finished execution, release it to pool
			if (VM && VMPool.IsValid())
			{
				VMPool->Release(VM);
			}
			
			ActiveVMs.RemoveAtSwap(i);
		}
	}

	// Update pool stats
	if (VMPool.IsValid())
	{
		int32 Total, Available, Active;
		VMPool->GetStats(Total, Available, Active);
		SET_DWORD_STAT(STAT_VMsPooled, Available);
	}

	int32 TotalBuffers, AvailableBuffers;
	FBytecodePool::GetStats(TotalBuffers, AvailableBuffers);
	SET_DWORD_STAT(STAT_BytecodeBuffersPooled, AvailableBuffers);
}

void UHktSimulationSubsystem::BuildBytecodeForEvent(FHktFlowVM& VM, const FHktIntentEvent& Event)
{
	SCOPE_CYCLE_COUNTER(STAT_BuildBytecode);

	FHktFlowBuilder B(VM.Bytecode);

	// [NEW] Try to use Flow Definition Registry first
	IFlowDefinition* FlowDef = nullptr;
	{
		SCOPE_CYCLE_COUNTER(STAT_FlowRegistryLookup);
		FlowDef = FFlowDefinitionRegistry::Find(Event.EventTag);
	}
	
	if (FlowDef)
	{
		// Validate event before building
		if (FlowDef->ValidateEvent(Event))
		{
			// Prepare registers for the flow definition
			if (Event.Target.IsValid())
			{
				FUnitHandle TargetHandle = GetOrCreateInternalHandle(Event.Target.Value, FVector::ZeroVector);
				VM.Regs.Get(1).SetUnit(TargetHandle);
			}
			
			// Set location in GPR[0]
			VM.Regs.Get(0).SetVector(Event.Location);

			// Build bytecode using the registered flow definition
			if (FlowDef->BuildBytecode(B, Event, &EntityManager))
			{
				UE_LOG(LogHktSimulation, Verbose, TEXT("Built bytecode using Flow Definition for tag: %s"), 
					*Event.EventTag.ToString());
				return; // Successfully built bytecode
			}
			else
			{
				UE_LOG(LogHktSimulation, Warning, TEXT("Flow Definition failed to build bytecode for tag: %s"), 
					*Event.EventTag.ToString());
			}
		}
		else
		{
			UE_LOG(LogHktSimulation, Warning, TEXT("Event validation failed for tag: %s"), 
				*Event.EventTag.ToString());
			return; // Don't fallback to legacy if validation fails
		}
	}

	// [LEGACY FALLBACK] Old hardcoded system (will be removed in Phase 3)
	FString TagString = Event.EventTag.ToString();
	
	if (TagString.Contains(TEXT("Attack")))
	{
		// 공격 이벤트 처리 예시
		// Target이 있으면 Target에게 데미지
		if (Event.Target.IsValid())
		{
			// Target의 내부 핸들 획득
			FUnitHandle TargetHandle = GetOrCreateInternalHandle(Event.Target.Value, FVector::ZeroVector);
			
			// GPR[1]에 타겟 설정
			VM.Regs.Get(1).SetUnit(TargetHandle);
			
			// 데미지 적용 (Magnitude를 데미지로 사용)
			float Damage = Event.Magnitude > 0.0f ? Event.Magnitude : 10.0f;
			B.ModifyHealth(1, -Damage);
		}
	}
	else if (TagString.Contains(TEXT("Move")))
	{
		// 이동 이벤트는 별도 처리 (MoveSpeed 기반 이동 시스템에서 처리)
		// 여기서는 위치 업데이트만 수행
		if (EntityManager.IsUnitValid(VM.Regs.OwnerUnit))
		{
			// 이동 목표 위치를 레지스터에 저장
			VM.Regs.Get(0).SetVector(Event.Location);
		}
	}
	else if (TagString.Contains(TEXT("Skill")))
	{
		// 스킬 이벤트 예시: Fireball
		constexpr uint8 RegProjectile = 0;
		
		B.PlayAnim(FName("Montage_Cast"))
		 .WaitSeconds(0.3f);
		
		// 스킬에 따른 추가 처리
		if (TagString.Contains(TEXT("Fireball")))
		{
			// Fireball 스킬: 투사체 생성 -> 충돌 대기 -> 폭발
			// ProjectileClass는 별도 시스템에서 관리
			// B.SpawnFireball(FireballClass, RegProjectile)
			//  .WaitForImpact(RegProjectile)
			//  .Explode(500.0f, 100.0f, 10.0f, 5.0f, RegProjectile);
		}
	}
	else if (TagString.Contains(TEXT("Spawn")))
	{
		// 스폰 이벤트: 새 유닛 생성은 이미 GetOrCreateInternalHandle에서 처리됨
		// 추가 초기화가 필요하면 여기서 수행
	}
	
	// 바이트코드가 비어있으면 기본 no-op 추가 방지
	// (빈 바이트코드는 VM 실행 큐에 추가하지 않음)
}

// ============================================================================
// IHktPlayerAttributeProvider 구현
// ============================================================================

FOnPlayerAttributeChanged& UHktSimulationSubsystem::OnPlayerAttributeChanged()
{
	return OnPlayerAttributeChangedDelegate;
}

bool UHktSimulationSubsystem::GetPlayerAttributeSnapshot(const FHktPlayerHandle& PlayerHandle, FHktPlayerAttributeSnapshot& OutSnapshot) const
{
	if (!PlayerHandle.IsValid())
	{
		return false;
	}

	const int32 PlayerIndex = PlayerHandle.Value;
	
	if (!EntityManager.Players.Attributes.IsValidIndex(PlayerIndex) ||
		!EntityManager.Players.IsActive[PlayerIndex])
	{
		return false;
	}

	ConvertToSnapshot(PlayerIndex, EntityManager.Players.Attributes[PlayerIndex], OutSnapshot);
	OutSnapshot.PlayerHandle = PlayerHandle;
	
	return true;
}

bool UHktSimulationSubsystem::ConsumeChangedPlayers(TArray<FHktPlayerAttributeSnapshot>& OutSnapshots)
{
	OutSnapshots.Reset();

	const int32 NumPlayers = EntityManager.Players.Attributes.Num();
	
	for (int32 i = 0; i < NumPlayers; ++i)
	{
		// Fast check: skip if not dirty
		if (!EntityManager.Players.IsDirty(i))
		{
			continue;
		}

		// Skip if not active
		if (!EntityManager.Players.IsActive.IsValidIndex(i) || !EntityManager.Players.IsActive[i])
		{
			EntityManager.Players.ClearDirty(i);
			continue;
		}

		// Create snapshot
		FHktPlayerAttributeSnapshot Snapshot;
		Snapshot.PlayerHandle.Value = i;
		ConvertToSnapshot(i, EntityManager.Players.Attributes[i], Snapshot);
		
		OutSnapshots.Add(MoveTemp(Snapshot));

		// Clear dirty flag
		EntityManager.Players.ClearDirty(i);
	}

	return OutSnapshots.Num() > 0;
}

FHktPlayerHandle UHktSimulationSubsystem::RegisterPlayer()
{
	const int32 PlayerIndex = EntityManager.AllocPlayer();
	
	FHktPlayerHandle Handle;
	Handle.Value = PlayerIndex;
	
	UE_LOG(LogHktSimulation, Log, TEXT("Registered new player with Handle: %d"), Handle.Value);
	
	return Handle;
}

void UHktSimulationSubsystem::UnregisterPlayer(const FHktPlayerHandle& PlayerHandle)
{
	if (!PlayerHandle.IsValid())
	{
		return;
	}

	EntityManager.FreePlayer(FPlayerHandle(PlayerHandle.Value));
	
	UE_LOG(LogHktSimulation, Log, TEXT("Unregistered player with Handle: %d"), PlayerHandle.Value);
}

void UHktSimulationSubsystem::ConvertToSnapshot(int32 PlayerIndex, const FHktAttributeSet& Attrs, FHktPlayerAttributeSnapshot& OutSnapshot) const
{
	OutSnapshot.AllAttributes.SetNumZeroed(static_cast<int32>(EHktAttributeType::Count));
	
	// EHktAttribute (Simulation internal) → EHktAttributeType (Service interface)
	// 두 enum이 동일한 순서로 정의되어 있다고 가정
	for (int32 i = 0; i < static_cast<int32>(EHktAttribute::Count); ++i)
	{
		OutSnapshot.AllAttributes[i] = Attrs.Values[i];
	}
}
