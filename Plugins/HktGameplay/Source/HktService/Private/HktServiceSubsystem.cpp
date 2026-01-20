#include "HktServiceSubsystem.h"
#include "Engine/World.h"

void UHktServiceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UHktServiceSubsystem::Deinitialize()
{
	SelectionProvider = nullptr;
	IntentEventProvider = nullptr;
	AssetProvider = nullptr;
	JobProvider = nullptr;
	Super::Deinitialize();
}

UHktServiceSubsystem* UHktServiceSubsystem::Get(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return World->GetSubsystem<UHktServiceSubsystem>();
	}
	return nullptr;
}

void UHktServiceSubsystem::RegisterSelectionProvider(TScriptInterface<IHktSelectionProvider> Provider)
{
	SelectionProvider = Provider;
}

void UHktServiceSubsystem::UnregisterSelectionProvider(TScriptInterface<IHktSelectionProvider> Provider)
{
	if (SelectionProvider == Provider)
	{
		SelectionProvider = nullptr;
	}
}

void UHktServiceSubsystem::RegisterIntentEventProvider(TScriptInterface<IHktIntentEventProvider> Provider)
{
	IntentEventProvider = Provider;
}

void UHktServiceSubsystem::UnregisterIntentEventProvider(TScriptInterface<IHktIntentEventProvider> Provider)
{
	if (IntentEventProvider == Provider)
	{
		IntentEventProvider = nullptr;
	}
}

void UHktServiceSubsystem::RegisterAssetProvider(TScriptInterface<IHktAssetProvider> Provider)
{
	AssetProvider = Provider;
}

void UHktServiceSubsystem::UnregisterAssetProvider(TScriptInterface<IHktAssetProvider> Provider)
{
	if (AssetProvider == Provider)
	{
		AssetProvider = nullptr;
	}
}

TScriptInterface<IHktSelectionProvider> UHktServiceSubsystem::GetSelectionProvider() const
{
	return SelectionProvider;
}

TScriptInterface<IHktIntentEventProvider> UHktServiceSubsystem::GetIntentEventProvider() const
{
	return IntentEventProvider;
}

TScriptInterface<IHktAssetProvider> UHktServiceSubsystem::GetAssetProvider() const
{
	return AssetProvider;
}

void UHktServiceSubsystem::RegisterJobProvider(TScriptInterface<IHktJobProvider> Provider)
{
	JobProvider = Provider;
}

void UHktServiceSubsystem::UnregisterJobProvider(TScriptInterface<IHktJobProvider> Provider)
{
	if (JobProvider == Provider)
	{
		JobProvider = nullptr;
	}
}

TScriptInterface<IHktJobProvider> UHktServiceSubsystem::GetJobProvider() const
{
	return JobProvider;
}
