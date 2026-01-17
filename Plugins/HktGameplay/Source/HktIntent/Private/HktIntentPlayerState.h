#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTagContainer.h"
#include "HktIntentPlayerState.generated.h"

class UHktInputContext;
class UHktIntentEventComponent;
class UHktIntentBuilderComponent;

/**
 * PlayerState for HktIntent system.
 * Owns IntentEventComponent for network replication.
 */
UCLASS()
class HKTINTENT_API AHktIntentPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AHktIntentPlayerState();

	//-------------------------------------------------------------------------
	// Intent Submission (called by PlayerController)
	//-------------------------------------------------------------------------
	
	/** Builder로부터 Intent를 구성하여 서버로 전송 */
	void SubmitIntent(UHktIntentBuilderComponent* Builder);

protected:
	virtual void BeginPlay() override;

private:
	/** Intent 전송 컴포넌트 (네트워크 복제 담당) */
	UPROPERTY(VisibleAnywhere, Category = "Hkt|Components")
	TObjectPtr<UHktIntentEventComponent> IntentEventComponent;

	/** Player Handle */
	UPROPERTY(Replicated)
	FHktPlayerHandle PlayerHandle;
};
