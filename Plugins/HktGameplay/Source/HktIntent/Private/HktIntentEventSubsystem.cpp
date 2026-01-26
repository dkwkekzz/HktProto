#include "HktIntentEventSubsystem.h"
#include "HktServiceSubsystem.h"

void UHktIntentEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UHktIntentEventSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

UHktIntentEventSubsystem* UHktIntentEventSubsystem::Get(const UObject* WorldContextObject)
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
    {
        return World->GetSubsystem<UHktIntentEventSubsystem>();
    }

    return nullptr;
}

void UHktIntentEventSubsystem::RegisterIntentEventComponent(UHktIntentEventComponent* Component)
{
    IntentEventComponents.Add(Component);
}

void UHktIntentEventSubsystem::UnregisterIntentEventComponent(UHktIntentEventComponent* Component)
{
    IntentEventComponents.Remove(Component);
}

void UHktIntentEventSubsystem::PushIntentBatch(int32 FrameNumber)
{
    for (UHktIntentEventComponent* Component : IntentEventComponents)
    {
        Component->PushIntentBatch(FrameNumber);
    }
}

void UHktIntentEventSubsystem::PullIntentEvents(int32 CompletedFrameNumber, TArray<FHktIntentEvent>& OutIntentEvents)
{
    for (UHktIntentEventComponent* Component : IntentEventComponents)
    {
        Component->PullIntentEvents(CompletedFrameNumber, OutIntentEvents);
    }
}