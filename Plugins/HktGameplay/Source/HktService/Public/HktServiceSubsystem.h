#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "HktServiceInterface.h"
#include "HktAssetInterface.h"
#include "HktIntentInterface.h"
#include "HktSimulationInterface.h"
#include "IHktJobProvider.h"
#include "IHktSelectionProvider.h"
#include "IHktPlayerAttributeProvider.h"
#include "HktServiceSubsystem.generated.h"

/**
 * Central hub for accessing services across modules.
 * Manages providers for Selection, IntentEvents, and Codex.
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

	void RegisterIntentEventProvider(TScriptInterface<IHktIntentEventProvider> Provider);
	void UnregisterIntentEventProvider(TScriptInterface<IHktIntentEventProvider> Provider);

	void RegisterAssetProvider(TScriptInterface<IHktAssetProvider> Provider);
	void UnregisterAssetProvider(TScriptInterface<IHktAssetProvider> Provider);

	void RegisterJobProvider(TScriptInterface<IHktJobProvider> Provider);
	void UnregisterJobProvider(TScriptInterface<IHktJobProvider> Provider);

	void RegisterPlayerAttributeProvider(TScriptInterface<IHktPlayerAttributeProvider> Provider);
	void UnregisterPlayerAttributeProvider(TScriptInterface<IHktPlayerAttributeProvider> Provider);

	// Accessors
	TScriptInterface<IHktSelectionProvider> GetSelectionProvider() const;
	TScriptInterface<IHktIntentEventProvider> GetIntentEventProvider() const;
	TScriptInterface<IHktAssetProvider> GetAssetProvider() const;
	TScriptInterface<IHktJobProvider> GetJobProvider() const;
	TScriptInterface<IHktPlayerAttributeProvider> GetPlayerAttributeProvider() const;

private:
	UPROPERTY()
	TScriptInterface<IHktSelectionProvider> SelectionProvider;

	UPROPERTY()
	TScriptInterface<IHktIntentEventProvider> IntentEventProvider;

	UPROPERTY()
	TScriptInterface<IHktAssetProvider> AssetProvider;

	UPROPERTY()
	TScriptInterface<IHktJobProvider> JobProvider;

	UPROPERTY()
	TScriptInterface<IHktPlayerAttributeProvider> PlayerAttributeProvider;
};
