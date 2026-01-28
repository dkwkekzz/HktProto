#include "HktSimulationSubsystem.h"

void UHktSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UHktSimulationSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

UHktSimulationSubsystem* UHktSimulationSubsystem::Get(const UObject* WorldContextObject)
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
    {
        return World->GetSubsystem<UHktSimulationSubsystem>();
    }

    return nullptr;
}

void UHktSimulationSubsystem::Execute(const FHktIntentEvent& Event)
{
    VMProcessor.QueueIntentEvent(Event);
}