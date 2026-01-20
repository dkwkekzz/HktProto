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

	// HktServiceSubsystem 의존성 초기화
	Collection.InitializeDependency<UHktServiceSubsystem>();

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
	
	// Sink에 알림
	if (IHktAttributeSink* Sink = GetAttributeSink())
	{
		Sink->OnPlayerRegistered(Handle);
		
		// 초기 속성 전송
		TArray<float> InitialValues;
		InitialValues.SetNumZeroed(static_cast<int32>(EHktAttributeType::Count));
		const FHktAttributeSet& Attrs = EntityManager.Players.Attributes[PlayerIndex];
		for (int32 i = 0; i < static_cast<int32>(EHktAttribute::Count); ++i)
		{
			InitialValues[i] = Attrs.Values[i];
		}
		Sink->PushAllAttributes(Handle, InitialValues);
	}
	
	UE_LOG(LogHktSimulation, Log, TEXT("Registered player with Handle: %d"), Handle.Value);
	return Handle;
}

void UHktSimulationSubsystem::UnregisterPlayer(const FHktPlayerHandle& PlayerHandle)
{
	if (!PlayerHandle.IsValid())
	{
		return;
	}

	// Sink에 알림
	if (IHktAttributeSink* Sink = GetAttributeSink())
	{
		Sink->OnPlayerUnregistered(PlayerHandle);
	}

	EntityManager.FreePlayer(FPlayerHandle(PlayerHandle.Value));
	UE_LOG(LogHktSimulation, Log, TEXT("Unregistered player with Handle: %d"), PlayerHandle.Value);
}

// ============================================================================
// Player Attribute API - 즉시 Sink에 전달
// ============================================================================

void UHktSimulationSubsystem::SetPlayerAttribute(const FHktPlayerHandle& Handle, EHktAttributeType Type, float Value)
{
	if (!Handle.IsValid())
	{
		return;
	}

	// 내부 DB 업데이트
	const int32 PlayerIndex = Handle.Value;
	if (EntityManager.Players.Attributes.IsValidIndex(PlayerIndex) && EntityManager.Players.IsActive[PlayerIndex])
	{
		EntityManager.Players.Attributes[PlayerIndex].Set(static_cast<EHktAttribute>(Type), Value);
		
		// Sink에 즉시 전달
		if (IHktAttributeSink* Sink = GetAttributeSink())
		{
			Sink->PushAttribute(Handle, Type, Value);
		}
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
		// 내부 DB 수정
		FHktAttributeSet& Attrs = EntityManager.Players.Attributes[PlayerIndex];
		Attrs.Modify(static_cast<EHktAttribute>(Type), Delta);
		
		// 새 값을 Sink에 즉시 전달
		const float NewValue = Attrs.Get(static_cast<EHktAttribute>(Type));
		if (IHktAttributeSink* Sink = GetAttributeSink())
		{
			Sink->PushAttribute(Handle, Type, NewValue);
		}
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

	TArray<FHktIntentEvent> NewEvents;
	if (!Provider->FetchNewEvents(LastProcessedFrame, NewEvents))
	{
		return;
	}

	for (const FHktIntentEvent& Event : NewEvents)
	{
		ExecuteIntentEvent(Event);
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
