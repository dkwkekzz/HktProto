#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTagContainer.h"
#include "HktServiceTypes.h"
#include "Objects/HktInputContexts.h"
#include "HktIntentPlayerState.generated.h"

/**
 * PlayerState for HktIntent system.
 * Manages client-local state such as Input Mappings (Key -> Action).
 */
UCLASS()
class HKTINTENT_API AHktIntentPlayerState : public APlayerState, public IHktSubjectContext, public IHktCommandContext
{
	GENERATED_BODY()

public:
	AHktIntentPlayerState();

protected:
	virtual void BeginPlay() override;

	// IHktSubjectContext Interface
	virtual TArray<FHktUnitHandle> ResolveSubjects() const override;
	virtual FHktUnitHandle ResolvePrimarySubject() const override;
	virtual bool IsValid() const override;
	virtual bool IsPrimarySubject() const override { return true; }

	// IHktCommandContext Interface
	virtual FGameplayTag ResolveEventTag() const override;
	virtual bool IsRequiredTarget() const override { return false; }

private:
	/** Player Handle */
	UPROPERTY(Transient)
	FHktUnitHandle PlayerHandle;

	UPROPERTY(Transient)
	FGameplayTag PlayerInitialEventTag;
};
