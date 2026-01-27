#include "HktSimulationSubsystem.h"
#include "HktServiceSubsystem.h"
#include "HktSimulationStashComponent.h"
#include "Build/HktFlowBuildProcessor.h"
#include "Core/HktVMRuntime.h"
#include "Processors/HktWaitProcessor.h"
#include "Processors/HktExecuteProcessor.h"
#include "Processors/HktCleanupProcessor.h"
#include "Engine/World.h"

// ============================================================================
// 라이프사이클
// ============================================================================

void UHktSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("[HktSimulation] Subsystem Initialized - JobQueue/WaitQueue Architecture Active"));
}

void UHktSimulationSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UHktSimulationSubsystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    for (FHktSimulationContext& Context : SimulationContexts)
    {
        TArray<FHktIntentEvent> IntentEvents;
        Context.IntentEventProvider->PullIntentEvents(CompletedFrameNumber, IntentEvents);
        for (const FHktIntentEvent& IntentEvent : IntentEvents)
        {
            FIntentEvent CoreEvent;
            CoreEvent.FrameNumber = IntentEvent.FrameNumber;
            CoreEvent.SourceEntity = IntentEvent.Subject.Value;
            CoreEvent.EventTag = IntentEvent.EventTag;
            CoreEvent.TargetEntity = IntentEvent.Target.Value;
            CoreEvent.TargetLocation = IntentEvent.TargetLocation;
            Context.VMProcessor.QueueIntentEvent(CoreEvent);
        }
        
        Context.VMProcessor.Tick(CompletedFrameNumber, DeltaTime);
    }
}

TStatId UHktSimulationSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UHktSimulationSubsystem, STATGROUP_Tickables);
}

UHktSimulationSubsystem* UHktSimulationSubsystem::Get(const UObject* WorldContextObject)
{
    if (!WorldContextObject) return nullptr;
    
    UWorld* World = WorldContextObject->GetWorld();
    if (!World) return nullptr;
    
    return World->GetSubsystem<UHktSimulationSubsystem>();
}

void UHktSimulationSubsystem::RegisterStashComponent(UHktSimulationStashComponent* Component)
{
    if (!Component) return;
    
    FHktSimulationContext Context;
    Context.StashComponent = Component;

    // IntentEventProvider 설정
    TArray<UActorComponent*> Components;
    Component->GetOwner()->GetComponents(Components);
    for (UActorComponent* Comp : Components)
    {
        if (IHktIntentEventProvider* Accessor = Cast<IHktIntentEventProvider>(Comp))
        {
            Context.IntentEventProvider = Component->IntentEventProvider;
            break;
        }
    }

    Context.VMProcessor.Initialize(GetWorld(), Component->GetStash());

    SimulationContexts.Add(Context);
}

void UHktSimulationSubsystem::UnregisterStashComponent(UHktSimulationStashComponent* Component)
{
    for (int32 i = 0; i < SimulationContexts.Num(); i++)
    {
        if (SimulationContexts[i].StashComponent == Component)
        {
            SimulationContexts.RemoveAtSwap(i);
            return;
        }
    }
}
