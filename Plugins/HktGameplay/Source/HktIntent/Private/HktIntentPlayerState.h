#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTagContainer.h"
#include "HktServiceInterface.h"
#include "HktIntentPlayerState.generated.h"

class UHktInputContext;
class UHktIntentEventComponent;
class UHktIntentBuilderComponent;

/**
 * PlayerState for HktIntent system.
 * 
 * Components:
 * - IntentEventComponent: 클라이언트 인텐트를 서버로 전송
 * 
 * Lockstep 동기화:
 * - Late Join 클라이언트는 서버로부터 스냅샷 RPC를 받아 SimulationSubsystem 초기화
 * - 정상 클라이언트는 로컬 Simulation 결과 사용
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
	// Player Handle
	//-------------------------------------------------------------------------
	
	/** PlayerHandle 접근자 */
	UFUNCTION(BlueprintPure, Category = "Hkt|Player")
	FHktPlayerHandle GetPlayerHandle() const { return PlayerHandle; }

	/** PlayerHandle 설정 (Server only, GameMode에서 호출) */
	void SetPlayerHandle(const FHktPlayerHandle& InHandle);

	//-------------------------------------------------------------------------
	// Late Join Snapshot (Server → Client RPC)
	//-------------------------------------------------------------------------
	
	/** 서버에서 Late Join 클라이언트에게 스냅샷 전송 */
	UFUNCTION(Client, Reliable)
	void Client_InitializeSnapshot(const TArray<float>& AttributeValues);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	/** Intent 전송 컴포넌트 (네트워크 복제 담당) */
	UPROPERTY(VisibleAnywhere, Category = "Hkt|Components")
	TObjectPtr<UHktIntentEventComponent> IntentEventComponent;

	/** Player Handle (Simulation과 연결) */
	UPROPERTY(Replicated)
	FHktPlayerHandle PlayerHandle;
};
