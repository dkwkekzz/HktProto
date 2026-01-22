#include "HktServiceSubsystem.h"
#include "Engine/World.h"

void UHktServiceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UHktServiceSubsystem::Deinitialize()
{
	SelectionProvider = nullptr;
	AssetProvider = nullptr;
	JobProvider = nullptr;
	SimulationProvider = nullptr;
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

void UHktServiceSubsystem::RegisterSimulationProvider(TScriptInterface<IHktSimulationProvider> Provider)
{
	SimulationProvider = Provider;
}

void UHktServiceSubsystem::UnregisterSimulationProvider(TScriptInterface<IHktSimulationProvider> Provider)
{
	if (SimulationProvider == Provider)
	{
		SimulationProvider = nullptr;
	}
}

TScriptInterface<IHktSimulationProvider> UHktServiceSubsystem::GetSimulationProvider() const
{
	return SimulationProvider;
}
