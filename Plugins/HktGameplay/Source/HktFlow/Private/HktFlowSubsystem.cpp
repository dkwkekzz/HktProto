// Copyright (c) 2026 Hkt Games. All Rights Reserved.

#include "HktFlowSubsystem.h"
#include "HktFlowBuilder.h"
#include "HktIntentInterface.h"
#include "HktServiceSubsystem.h"

// ============================================================================
// FHktFlowRegistry Implementation
// ============================================================================
void FHktFlowRegistry::RegisterFlow(FGameplayTag Tag, TSharedPtr<IHktFlow> FlowInstance)
{
    UHktFlowSubsystem::RegisterFlowType(Tag, FlowInstance);
}

// ============================================================================
// UHktFlowSubsystem Implementation
// ============================================================================

DEFINE_LOG_CATEGORY(LogHktFlow);

TMap<FGameplayTag, TSharedPtr<IHktFlow>>& UHktFlowSubsystem::GetGlobalFlowRegistry()
{
    static TMap<FGameplayTag, TSharedPtr<IHktFlow>> Registry;
    return Registry;
}

void UHktFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (UHktServiceSubsystem* Service = Collection.InitializeDependency<UHktServiceSubsystem>())
    {
        Service->RegisterJobProvider(this);
    }
}

void UHktFlowSubsystem::Deinitialize()
{
    if (UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld()))
    {
        Service->UnregisterJobProvider(this);
    }

    Super::Deinitialize();
}

void UHktFlowSubsystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateFlows(DeltaTime);
}

TStatId UHktFlowSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UHktFlowSubsystem, STATGROUP_Tickables);
}

void UHktFlowSubsystem::RegisterFlowInstance(FGameplayTag Tag, TSharedPtr<IHktFlow> FlowInstance)
{
    if (Tag.IsValid() && FlowInstance.IsValid())
    {
        GetGlobalFlowRegistry().Add(Tag, FlowInstance);
    }
}

TSharedPtr<IHktFlow> UHktFlowSubsystem::GetFlow(FGameplayTag Tag)
{
    if (TSharedPtr<IHktFlow>* FoundFlow = GetGlobalFlowRegistry().Find(Tag))
    {
        return *FoundFlow;
    }
    return nullptr;
}

void UHktFlowSubsystem::UpdateFlows(float DeltaTime)
{
    // NOTE: IntentEventProvider has been removed.
    // Flow processing is now handled by UHktSimulationProcessSubsystem
    // which iterates through UHktSimulationStashComponent instances.
    // 
    // This function is kept for potential future use with direct Flow registration.
}
