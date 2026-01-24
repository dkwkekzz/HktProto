#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "HktServiceInterface.h"
#include "HktAssetInterface.h"
#include "HktSimulationInterface.h"
#include "IHktSelectionProvider.h"
#include "HktServiceSubsystem.generated.h"

/**
 * Central hub for accessing services across modules.
 * Manages providers for Selection, Simulation, Asset, and Job.
 * 
 * Note: IntentEventProvider has been removed.
 * IntentEvents are now handled directly by UHktIntentEventComponent
 * and passed to UHktSimulationStashComponent for processing.
 */
UCLASS()
class HKTSERVICE_API UHktServiceSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Helper to get the subsystem from a world context
	static UHktServiceSubsystem* Get(const UObject* WorldContextObject);

	// Registration
	void RegisterSelectionProvider(TScriptInterface<IHktSelectionProvider> Provider);
	void UnregisterSelectionProvider(TScriptInterface<IHktSelectionProvider> Provider);

	void RegisterAssetProvider(TScriptInterface<IHktAssetProvider> Provider);
	void UnregisterAssetProvider(TScriptInterface<IHktAssetProvider> Provider);

	void RegisterIntentEventProvider(TScriptInterface<IHktIntentEventProvider> Provider);
	void UnregisterIntentEventProvider(TScriptInterface<IHktIntentEventProvider> Provider);

	// Accessors
	TScriptInterface<IHktSelectionProvider> GetSelectionProvider() const;
	TScriptInterface<IHktAssetProvider> GetAssetProvider() const;
	const TArray<TScriptInterface<IHktIntentEventProvider>>& GetIntentEventProviders() const;
	TScriptInterface<IHktIntentEventProvider> GetIntentEventProvider(int32 Index) const;

private:
	UPROPERTY()
	TScriptInterface<IHktSelectionProvider> SelectionProvider;

	UPROPERTY()
	TScriptInterface<IHktAssetProvider> AssetProvider;
	
	UPROPERTY()
	TArray<TScriptInterface<IHktIntentEventProvider>> IntentEventProviders;
};
