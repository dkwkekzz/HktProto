#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTagContainer.h"
#include "HktServiceInterface.h"
#include "HktIntentPlayerState.generated.h"

class UHktInputContext;
class UHktIntentEventComponent;
class UHktIntentBuilderComponent;
class UHktAttributeComponent;

/**
 * PlayerState for HktIntent system.
 * 
 * Components:
 * - IntentEventComponent: 클라이언트 인텐트를 서버로 전송
 * - AttributeComponent: 플레이어 속성을 FFastArraySerializer로 리플리케이션
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

	//-------------------------------------------------------------------------
	// Attribute Access
	//-------------------------------------------------------------------------
	
	/** AttributeComponent 접근자 */
	UFUNCTION(BlueprintPure, Category = "Hkt|Attributes")
	UHktAttributeComponent* GetAttributeComponent() const { return AttributeComponent; }

	/** PlayerHandle 접근자 */
	UFUNCTION(BlueprintPure, Category = "Hkt|Player")
	FHktPlayerHandle GetPlayerHandle() const { return PlayerHandle; }

	/** PlayerHandle 설정 (Server only, IntentSubsystem에서 호출) */
	void SetPlayerHandle(const FHktPlayerHandle& InHandle);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	/** Intent 전송 컴포넌트 (네트워크 복제 담당) */
	UPROPERTY(VisibleAnywhere, Category = "Hkt|Components")
	TObjectPtr<UHktIntentEventComponent> IntentEventComponent;

	/** 플레이어 속성 컴포넌트 (FFastArraySerializer 기반 리플리케이션) */
	UPROPERTY(VisibleAnywhere, Category = "Hkt|Components")
	TObjectPtr<UHktAttributeComponent> AttributeComponent;

	/** Player Handle (Simulation과 연결) */
	UPROPERTY(Replicated)
	FHktPlayerHandle PlayerHandle;
};
