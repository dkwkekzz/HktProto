#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "HktServiceInterfaces.h"
#include "IHktCodexProvider.h"
#include "IHktSelectionProvider.h"
#include "IHktIntentEventProvider.h"
#include "IHktJobProvider.h"
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

	void RegisterCodexProvider(TScriptInterface<IHktCodexProvider> Provider);
	void UnregisterCodexProvider(TScriptInterface<IHktCodexProvider> Provider);

	void RegisterJobProvider(TScriptInterface<IHktJobProvider> Provider);
	void UnregisterJobProvider(TScriptInterface<IHktJobProvider> Provider);

	// Accessors
	TScriptInterface<IHktSelectionProvider> GetSelectionProvider() const;
	TScriptInterface<IHktIntentEventProvider> GetIntentEventProvider() const;
	TScriptInterface<IHktCodexProvider> GetCodexProvider() const;
	TScriptInterface<IHktJobProvider> GetJobProvider() const;

private:
	UPROPERTY()
	TScriptInterface<IHktSelectionProvider> SelectionProvider;

	UPROPERTY()
	TScriptInterface<IHktIntentEventProvider> IntentEventProvider;

	UPROPERTY()
	TScriptInterface<IHktCodexProvider> CodexProvider;

	UPROPERTY()
	TScriptInterface<IHktJobProvider> JobProvider;
};
