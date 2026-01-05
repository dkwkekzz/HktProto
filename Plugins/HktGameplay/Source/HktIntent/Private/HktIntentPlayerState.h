#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTagContainer.h"
#include "HktServiceTypes.h"
#include "HktIntentPlayerState.generated.h"

class IHktSubjectContext;
class IHktCommandContext;

/**
 * PlayerState for HktIntent system.
 * Manages client-local state such as Input Mappings (Key -> Action).
 */
UCLASS()
class HKTINTENT_API AHktIntentPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AHktIntentPlayerState();
	~AHktIntentPlayerState();

	IHktSubjectContext* GetSubjectContext() const;
	IHktCommandContext* GetCommandContext() const;

protected:
	virtual void BeginPlay() override;

private:
	/** Player Handle */
	UPROPERTY(Transient)
	FHktUnitHandle PlayerHandle;

	UPROPERTY(Transient)
	FGameplayTag PlayerInitialEventTag;

	/** Subject Context Implementation (owned by this PlayerState) */
	TUniquePtr<IHktSubjectContext> SubjectContextImpl;

	/** Command Context Implementation (owned by this PlayerState) */
	TUniquePtr<IHktCommandContext> CommandContextImpl;
};
