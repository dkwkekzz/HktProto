// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktSimulationSubsystem.h"
#include "HktSimulationStashComponent.h"
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

void UHktSimulationProcessSubsystem::Initialize(FSubsystemCollectionBase& Collection)
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

	UE_LOG(LogHktSimulation, Log, TEXT("HktSimulationProcessSubsystem Initialized"));
}

void UHktSimulationProcessSubsystem::Deinitialize()
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
	
	// Clear registered components
	RegisteredStashComponents.Empty();
	
	VMPool.Reset();
	FBytecodePool::Clear();
	
	ExternalToInternalMap.Empty();
	LastProcessedFrameNumber = 0;
	bSimulationRunning = false;

	UE_LOG(LogHktSimulation, Log, TEXT("HktSimulationProcessSubsystem Deinitialized"));

	Super::Deinitialize();
}

void UHktSimulationProcessSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. 모든 StashComponent를 순회하며 이벤트 처리
	ProcessStashComponents(DeltaTime);

	// 2. 활성 VM들 틱
	TickActiveVMs(DeltaTime);

	// 3. 완료된 VM 정리
	CleanupFinishedVMs();
}

TStatId UHktSimulationProcessSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UHktSimulationProcessSubsystem, STATGROUP_Tickables);
}

bool UHktSimulationProcessSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

UHktSimulationProcessSubsystem* UHktSimulationProcessSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	return World ? World->GetSubsystem<UHktSimulationProcessSubsystem>() : nullptr;
}

// ============================================================================
// StashComponent Management
// ============================================================================

void UHktSimulationProcessSubsystem::RegisterStashComponent(UHktSimulationStashComponent* StashComponent)
{
	if (!StashComponent)
	{
		return;
	}
	
	// 중복 등록 방지
	if (RegisteredStashComponents.ContainsByPredicate([StashComponent](const TWeakObjectPtr<UHktSimulationStashComponent>& Ptr) {
		return Ptr.Get() == StashComponent;
	}))
	{
		return;
	}
	
	RegisteredStashComponents.Add(StashComponent);
	
	UE_LOG(LogHktSimulation, Log, TEXT("RegisterStashComponent: Total=%d"), RegisteredStashComponents.Num());
}

void UHktSimulationProcessSubsystem::UnregisterStashComponent(UHktSimulationStashComponent* StashComponent)
{
	if (!StashComponent)
	{
		return;
	}
	
	RegisteredStashComponents.RemoveAll([StashComponent](const TWeakObjectPtr<UHktSimulationStashComponent>& Ptr) {
		return !Ptr.IsValid() || Ptr.Get() == StashComponent;
	});
	
	UE_LOG(LogHktSimulation, Log, TEXT("UnregisterStashComponent: Total=%d"), RegisteredStashComponents.Num());
}

// ============================================================================
// Player Management
// ============================================================================

FHktPlayerHandle UHktSimulationProcessSubsystem::RegisterPlayer()
{
	const int32 PlayerIndex = EntityManager.AllocPlayer();
	
	FHktPlayerHandle Handle;
	Handle.Value = PlayerIndex;
	
	// Simulation 실행 상태로 마크
	bSimulationRunning = true;
	
	UE_LOG(LogHktSimulation, Log, TEXT("Registered player with Handle: %d"), Handle.Value);
	return Handle;
}

void UHktSimulationProcessSubsystem::UnregisterPlayer(const FHktPlayerHandle& PlayerHandle)
{
	if (!PlayerHandle.IsValid())
	{
		return;
	}

	EntityManager.FreePlayer(FPlayerHandle(PlayerHandle.Value));
	UE_LOG(LogHktSimulation, Log, TEXT("Unregistered player with Handle: %d"), PlayerHandle.Value);
}

// ============================================================================
// Player Attribute API
// ============================================================================

void UHktSimulationProcessSubsystem::SetPlayerAttribute(const FHktPlayerHandle& Handle, EHktAttributeType Type, float Value)
{
	if (!Handle.IsValid())
	{
		return;
	}

	const int32 PlayerIndex = Handle.Value;
	if (EntityManager.Players.Attributes.IsValidIndex(PlayerIndex) && EntityManager.Players.IsActive[PlayerIndex])
	{
		EntityManager.Players.Attributes[PlayerIndex].Set(static_cast<EHktAttribute>(Type), Value);
	}
}

void UHktSimulationProcessSubsystem::ModifyPlayerAttribute(const FHktPlayerHandle& Handle, EHktAttributeType Type, float Delta)
{
	if (!Handle.IsValid())
	{
		return;
	}

	const int32 PlayerIndex = Handle.Value;
	if (EntityManager.Players.Attributes.IsValidIndex(PlayerIndex) && EntityManager.Players.IsActive[PlayerIndex])
	{
		FHktAttributeSet& Attrs = EntityManager.Players.Attributes[PlayerIndex];
		Attrs.Modify(static_cast<EHktAttribute>(Type), Delta);
	}
}

// ============================================================================
// IHktSimulationProvider - Snapshot
// ============================================================================

bool UHktSimulationProcessSubsystem::GetPlayerSnapshot(const FHktPlayerHandle& Handle, TArray<float>& OutValues) const
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

	const FHktAttributeSet& Attrs = EntityManager.Players.Attributes[PlayerIndex];
	OutValues.SetNum(static_cast<int32>(EHktAttribute::Count));
	for (int32 i = 0; i < static_cast<int32>(EHktAttribute::Count); ++i)
	{
		OutValues[i] = Attrs.Values[i];
	}

	return true;
}

void UHktSimulationProcessSubsystem::InitializePlayerFromSnapshot(const FHktPlayerHandle& Handle, const TArray<float>& Values)
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

	FHktAttributeSet& Attrs = EntityManager.Players.Attributes[PlayerIndex];
	for (int32 i = 0; i < Values.Num() && i < static_cast<int32>(EHktAttribute::Count); ++i)
	{
		Attrs.Set(static_cast<EHktAttribute>(i), Values[i]);
	}

	bSimulationRunning = true;
	
	UE_LOG(LogHktSimulation, Log, TEXT("Initialized Player %d from snapshot (%d attributes)"), PlayerIndex, Values.Num());
}

// ============================================================================
// Entity Management
// ============================================================================

FUnitHandle UHktSimulationProcessSubsystem::GetOrCreateInternalHandle(int32 ExternalUnitId, FVector Location, FRotator Rotation)
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

int32 UHktSimulationProcessSubsystem::GetExternalUnitId(FUnitHandle InternalHandle) const
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

void UHktSimulationProcessSubsystem::ExecuteIntentEvent(const FHktIntentEvent& Event)
{
	FUnitHandle SubjectHandle = GetOrCreateInternalHandle(Event.Subject.Value, Event.Location);
	FHktFlowVM* NewVM = VMPool->Acquire(GetWorld(), &EntityManager, SubjectHandle);

	BuildBytecodeForEvent(*NewVM, Event);

	if (NewVM->Bytecode.Num() > 0)
	{
		ActiveVMs.Add(NewVM);
		UE_LOG(LogHktSimulation, Verbose, TEXT("Queued VM for IntentEvent: %s (Frame: %d, EventId: %d)"), 
			*Event.EventTag.ToString(), Event.FrameNumber, Event.EventId);
	}
	else
	{
		VMPool->Release(NewVM);
	}
}

// ============================================================================
// StashComponent Processing
// ============================================================================

void UHktSimulationProcessSubsystem::ProcessStashComponents(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_ProcessIntentEvents);

	// 무효한 참조 정리
	RegisteredStashComponents.RemoveAll([](const TWeakObjectPtr<UHktSimulationStashComponent>& Ptr) {
		return !Ptr.IsValid();
	});

	// 모든 StashComponent 순회
	for (const TWeakObjectPtr<UHktSimulationStashComponent>& StashPtr : RegisteredStashComponents)
	{
		if (UHktSimulationStashComponent* StashComponent = StashPtr.Get())
		{
			ProcessStashComponent(StashComponent);
		}
	}
}

void UHktSimulationProcessSubsystem::ProcessStashComponent(UHktSimulationStashComponent* StashComponent)
{
	if (!StashComponent)
	{
		return;
	}

	const TArray<FHktIntentEventBatch>& PendingBatches = StashComponent->GetPendingBatches();
	if (PendingBatches.Num() == 0)
	{
		return;
	}

	// 배치들을 프레임 순서대로 처리
	int32 ProcessedFrameNumber = 0;
	FHktSimulationResult CombinedResult;
	
	for (const FHktIntentEventBatch& Batch : PendingBatches)
	{
		// 캐시 적중률 최적화: 동일 타입 이벤트를 묶어서 처리
		ProcessBatch(Batch, CombinedResult);
		ProcessedFrameNumber = FMath::Max(ProcessedFrameNumber, Batch.FrameNumber);
		
		INC_DWORD_STAT_BY(STAT_EventsProcessed, Batch.Events.Num());
	}

	// 처리 완료 후 결과 저장 및 큐 정리
	if (ProcessedFrameNumber > 0)
	{
		StashComponent->StoreResult(ProcessedFrameNumber, CombinedResult);
		LastProcessedFrameNumber = FMath::Max(LastProcessedFrameNumber, ProcessedFrameNumber);
		
		UE_LOG(LogHktSimulation, Verbose, TEXT("ProcessStashComponent: Processed %d batches up to frame %d"), 
			PendingBatches.Num(), ProcessedFrameNumber);
	}
}

void UHktSimulationProcessSubsystem::ProcessBatch(const FHktIntentEventBatch& Batch, FHktSimulationResult& OutResult)
{
	// 캐시 적중률 최적화를 위해 동일 타입 이벤트를 그룹화
	// (현재는 순차 처리, 향후 SoA 구조 활용 가능)
	
	for (const FHktIntentEvent& Event : Batch.Events)
	{
		ExecuteIntentEvent(Event);
	}
	
	OutResult.ProcessedFrameNumber = Batch.FrameNumber;
}

void UHktSimulationProcessSubsystem::TickActiveVMs(float DeltaTime)
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

void UHktSimulationProcessSubsystem::CleanupFinishedVMs()
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

void UHktSimulationProcessSubsystem::BuildBytecodeForEvent(FHktFlowVM& VM, const FHktIntentEvent& Event)
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
