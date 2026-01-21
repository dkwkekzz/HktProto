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
		Service->RegisterSimulationProvider(this);
	}

	// Initialize VM pool
	VMPool = MakeUnique<FHktVMPool>();
	VMPool->SetMaxPoolSize(100);
	VMPool->Prewarm(10);

	// Prewarm bytecode pool
	FBytecodePool::Prewarm(20);

	UE_LOG(LogHktSimulation, Log, TEXT("HktSimulationSubsystem Initialized"));
}

void UHktSimulationSubsystem::Deinitialize()
{
	// Provider 해제
	if (UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld()))
	{
		Service->UnregisterSimulationProvider(this);
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
	LastProcessedEventId = 0;
	bSimulationRunning = false;
	CachedAttributeSink = nullptr;

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
	return World ? World->GetSubsystem<UHktSimulationSubsystem>() : nullptr;
}

// ============================================================================
// Player Management
// ============================================================================

FHktPlayerHandle UHktSimulationSubsystem::RegisterPlayer()
{
	const int32 PlayerIndex = EntityManager.AllocPlayer();
	
	FHktPlayerHandle Handle;
	Handle.Value = PlayerIndex;
	
	// Lockstep 방식: Sink 푸시 제거
	// 서버: FAS(AttributeComponent)로 초기 속성 복제
	// 클라이언트: Late Join 시 PostReplicatedAdd에서 스냅샷 수신
	
	// Simulation 실행 상태로 마크 (정상 Join)
	bSimulationRunning = true;
	
	UE_LOG(LogHktSimulation, Log, TEXT("Registered player with Handle: %d"), Handle.Value);
	return Handle;
}

void UHktSimulationSubsystem::UnregisterPlayer(const FHktPlayerHandle& PlayerHandle)
{
	if (!PlayerHandle.IsValid())
	{
		return;
	}

	// Lockstep 방식: Sink 알림 제거
	// 서버/클라이언트 모두 로컬에서만 해제

	EntityManager.FreePlayer(FPlayerHandle(PlayerHandle.Value));
	UE_LOG(LogHktSimulation, Log, TEXT("Unregistered player with Handle: %d"), PlayerHandle.Value);
}

// ============================================================================
// Player Attribute API - Lockstep 방식 (Sink 푸시 제거)
// ============================================================================

void UHktSimulationSubsystem::SetPlayerAttribute(const FHktPlayerHandle& Handle, EHktAttributeType Type, float Value)
{
	if (!Handle.IsValid())
	{
		return;
	}

	// 내부 DB만 업데이트 (클라이언트/서버 모두 동일하게 로컬 계산)
	const int32 PlayerIndex = Handle.Value;
	if (EntityManager.Players.Attributes.IsValidIndex(PlayerIndex) && EntityManager.Players.IsActive[PlayerIndex])
	{
		EntityManager.Players.Attributes[PlayerIndex].Set(static_cast<EHktAttribute>(Type), Value);
		
		// Sink 푸시 제거 - Late Join용 FAS 업데이트는 별도 처리
		// 서버: RegisterPlayer() 시 초기값만 설정, 이후 변경은 로컬 계산
		// 클라이언트: 로컬 계산 결과만 사용
	}
}

void UHktSimulationSubsystem::ModifyPlayerAttribute(const FHktPlayerHandle& Handle, EHktAttributeType Type, float Delta)
{
	if (!Handle.IsValid())
	{
		return;
	}

	const int32 PlayerIndex = Handle.Value;
	if (EntityManager.Players.Attributes.IsValidIndex(PlayerIndex) && EntityManager.Players.IsActive[PlayerIndex])
	{
		// 내부 DB만 수정
		FHktAttributeSet& Attrs = EntityManager.Players.Attributes[PlayerIndex];
		Attrs.Modify(static_cast<EHktAttribute>(Type), Delta);
		
		// Sink 푸시 제거
	}
}

IHktAttributeSink* UHktSimulationSubsystem::GetAttributeSink() const
{
	if (!CachedAttributeSink)
	{
		if (UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld()))
		{
			CachedAttributeSink = Service->GetAttributeSink().GetInterface();
		}
	}
	return CachedAttributeSink;
}

// ============================================================================
// IHktSimulationProvider - Snapshot
// ============================================================================

bool UHktSimulationSubsystem::GetPlayerSnapshot(const FHktPlayerHandle& Handle, TArray<float>& OutValues) const
{
	if (!Handle.IsValid())
	{
		return false;
	}

	const int32 PlayerIndex = Handle.Value;
	if (!EntityManager.Players.Attributes.IsValidIndex(PlayerIndex) || !EntityManager.Players.IsActive[PlayerIndex])
	{
		return false;
	}

	// 현재 속성 값 수집
	const FHktAttributeSet& Attrs = EntityManager.Players.Attributes[PlayerIndex];
	OutValues.SetNum(static_cast<int32>(EHktAttribute::Count));
	for (int32 i = 0; i < static_cast<int32>(EHktAttribute::Count); ++i)
	{
		OutValues[i] = Attrs.Values[i];
	}

	return true;
}

void UHktSimulationSubsystem::InitializePlayerFromSnapshot(const FHktPlayerHandle& Handle, const TArray<float>& Values)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogHktSimulation, Warning, TEXT("InitializePlayerFromSnapshot: Invalid Handle"));
		return;
	}

	const int32 PlayerIndex = Handle.Value;
	if (!EntityManager.Players.Attributes.IsValidIndex(PlayerIndex) || !EntityManager.Players.IsActive[PlayerIndex])
	{
		UE_LOG(LogHktSimulation, Warning, TEXT("InitializePlayerFromSnapshot: Player %d not valid"), PlayerIndex);
		return;
	}

	// 서버 스냅샷으로 로컬 DB 초기화
	FHktAttributeSet& Attrs = EntityManager.Players.Attributes[PlayerIndex];
	for (int32 i = 0; i < Values.Num() && i < static_cast<int32>(EHktAttribute::Count); ++i)
	{
		Attrs.Set(static_cast<EHktAttribute>(i), Values[i]);
	}

	// Simulation 실행 상태로 마크
	bSimulationRunning = true;
	
	UE_LOG(LogHktSimulation, Log, TEXT("Initialized Player %d from snapshot (%d attributes)"), PlayerIndex, Values.Num());
}

// ============================================================================
// Entity Management
// ============================================================================

FUnitHandle UHktSimulationSubsystem::GetOrCreateInternalHandle(int32 ExternalUnitId, FVector Location, FRotator Rotation)
{
	SCOPE_CYCLE_COUNTER(STAT_HandleMapping);

	if (FUnitHandle* Found = ExternalToInternalMap.Find(ExternalUnitId))
	{
		if (EntityManager.IsUnitValid(*Found))
		{
			return *Found;
		}
		ExternalToInternalMap.Remove(ExternalUnitId);
	}

	FPlayerHandle OwnerPlayer;
	FUnitHandle NewHandle = EntityManager.AllocUnit(OwnerPlayer, Location, Rotation);

	ExternalToInternalMap.Add(ExternalUnitId, NewHandle);
	EntityManager.Entities.ExternalIds[NewHandle.Index] = ExternalUnitId;

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
	if (EntityManager.IsUnitValid(InternalHandle))
	{
		return EntityManager.Entities.ExternalIds[InternalHandle.Index];
	}
	return INDEX_NONE;
}

// ============================================================================
// VM Management
// ============================================================================

void UHktSimulationSubsystem::ExecuteIntentEvent(const FHktIntentEvent& Event)
{
	FUnitHandle SubjectHandle = GetOrCreateInternalHandle(Event.Subject.Value, Event.Location);
	FHktFlowVM* NewVM = VMPool->Acquire(GetWorld(), &EntityManager, SubjectHandle);

	BuildBytecodeForEvent(*NewVM, Event);

	if (NewVM->Bytecode.Num() > 0)
	{
		ActiveVMs.Add(NewVM);
		UE_LOG(LogHktSimulation, Verbose, TEXT("Queued VM for IntentEvent: %s (EventId: %d)"), 
			*Event.EventTag.ToString(), Event.EventId);
	}
	else
	{
		VMPool->Release(NewVM);
	}
}

void UHktSimulationSubsystem::ProcessIntentEvents(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_ProcessIntentEvents);

	// Lockstep 방식: 서버/클라이언트 모두 동일하게 처리
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

	// Lockstep: Fetch()로 모든 이벤트를 가져와서 처리 (Flush됨)
	TArray<FHktIntentEvent> NewEvents;
	if (!Provider->Fetch(NewEvents))
	{
		return;
	}

	for (const FHktIntentEvent& Event : NewEvents)
	{
		ExecuteIntentEvent(Event);
		LastProcessedEventId = FMath::Max(LastProcessedEventId, Event.EventId);
		INC_DWORD_STAT(STAT_EventsProcessed);
	}

	// Commit은 서버만 호출 (EventBuffer 정리)
	if (NewEvents.Num() > 0)
	{
		if (GetWorld()->GetNetMode() != NM_Client)
		{
			FHktSimulationResult EmptyResult; // Lockstep: 속성 푸시 제거됨
			Provider->Commit(LastProcessedEventId, EmptyResult);
		}
		
		UE_LOG(LogHktSimulation, Verbose, TEXT("Processed %d events, LastEventId: %d"), 
			NewEvents.Num(), LastProcessedEventId);
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

	SET_DWORD_STAT(STAT_ActiveVMs, ActiveVMs.Num());
}

void UHktSimulationSubsystem::CleanupFinishedVMs()
{
	SCOPE_CYCLE_COUNTER(STAT_CleanupFinishedVMs);

	for (int32 i = ActiveVMs.Num() - 1; i >= 0; --i)
	{
		FHktFlowVM* VM = ActiveVMs[i];
		
		if (!VM || VM->Regs.ProgramCounter >= VM->Bytecode.Num())
		{
			if (VM && VMPool.IsValid())
			{
				VMPool->Release(VM);
			}
			ActiveVMs.RemoveAtSwap(i);
		}
	}

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

	IFlowDefinition* FlowDef = nullptr;
	{
		SCOPE_CYCLE_COUNTER(STAT_FlowRegistryLookup);
		FlowDef = FFlowDefinitionRegistry::Find(Event.EventTag);
	}
	
	if (FlowDef)
	{
		if (FlowDef->ValidateEvent(Event))
		{
			if (Event.Target.IsValid())
			{
				FUnitHandle TargetHandle = GetOrCreateInternalHandle(Event.Target.Value, FVector::ZeroVector);
				VM.Regs.Get(1).SetUnit(TargetHandle);
			}
			
			VM.Regs.Get(0).SetVector(Event.Location);

			if (FlowDef->BuildBytecode(B, Event, &EntityManager))
			{
				UE_LOG(LogHktSimulation, Verbose, TEXT("Built bytecode using Flow Definition for tag: %s"), 
					*Event.EventTag.ToString());
				return;
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
			return;
		}
	}

	// Legacy fallback
	FString TagString = Event.EventTag.ToString();
	
	if (TagString.Contains(TEXT("Attack")))
	{
		if (Event.Target.IsValid())
		{
			FUnitHandle TargetHandle = GetOrCreateInternalHandle(Event.Target.Value, FVector::ZeroVector);
			VM.Regs.Get(1).SetUnit(TargetHandle);
			
			float Damage = Event.Magnitude > 0.0f ? Event.Magnitude : 10.0f;
			B.ModifyHealth(1, -Damage);
		}
	}
	else if (TagString.Contains(TEXT("Move")))
	{
		if (EntityManager.IsUnitValid(VM.Regs.OwnerUnit))
		{
			VM.Regs.Get(0).SetVector(Event.Location);
		}
	}
	else if (TagString.Contains(TEXT("Skill")))
	{
		B.PlayAnim(FName("Montage_Cast"))
		 .WaitSeconds(0.3f);
	}
}
