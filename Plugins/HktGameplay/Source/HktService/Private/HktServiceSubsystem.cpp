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
	CodexProvider = nullptr;
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

void UHktServiceSubsystem::RegisterCodexProvider(TScriptInterface<IHktCodexProvider> Provider)
{
	CodexProvider = Provider;
}

void UHktServiceSubsystem::UnregisterCodexProvider(TScriptInterface<IHktCodexProvider> Provider)
{
	if (CodexProvider == Provider)
	{
		CodexProvider = nullptr;
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

TScriptInterface<IHktCodexProvider> UHktServiceSubsystem::GetCodexProvider() const
{
	return CodexProvider;
}
