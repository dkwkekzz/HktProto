#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTagContainer.h"
#include "HktServiceInterface.h"
#include "HktIntentPlayerState.generated.h"

class UHktInputContext;
class UHktIntentEventComponent;
class UHktIntentBuilderComponent;
class UHktSimulationStashComponent;

/**
 * PlayerState for HktIntent system.
 * 
 * Components:
 * - IntentEventComponent: Intent 네트워크 전송 및 배치 복제
 * - SimulationStashComponent: 이벤트 배치 큐 및 시뮬레이션 결과 관리
 * 
 * 데이터 흐름:
 * Builder → EventComponent → (RPC) → Server → StashComponent → SimulationSubsystem
 */
UCLASS()
class HKTINTENT_API AHktIntentPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AHktIntentPlayerState();

	//-------------------------------------------------------------------------
	// Player Handle
	//-------------------------------------------------------------------------
	
	/** PlayerHandle 접근자 */
	UFUNCTION(BlueprintPure, Category = "Hkt|Player")
	FHktPlayerHandle GetPlayerHandle() const { return PlayerHandle; }

	/** PlayerHandle 설정 (Server only, GameMode에서 호출) */
	void SetPlayerHandle(const FHktPlayerHandle& InHandle);

	//-------------------------------------------------------------------------
	// Component Accessors
	//-------------------------------------------------------------------------
	
	/** IntentEventComponent 접근자 */
	UHktIntentEventComponent* GetIntentEventComponent() const { return IntentEventComponent; }
	
	/** SimulationStashComponent 접근자 */
	UHktSimulationStashComponent* GetSimulationStashComponent() const { return SimulationStashComponent; }

	//-------------------------------------------------------------------------
	// Late Join Snapshot (Server → Client RPC)
	//-------------------------------------------------------------------------
	
	/** 서버에서 Late Join 클라이언트에게 스냅샷 전송 */
	UFUNCTION(Client, Reliable)
	void Client_InitializeSnapshot(const TArray<float>& AttributeValues);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	/** GameMode에 IntentEventComponent 등록 */
	void RegisterWithGameMode();
	
	/** GameMode에서 IntentEventComponent 등록 해제 */
	void UnregisterFromGameMode();

private:
	/** Intent 전송 컴포넌트 (네트워크 복제 담당) */
	UPROPERTY(VisibleAnywhere, Category = "Hkt|Components")
	TObjectPtr<UHktIntentEventComponent> IntentEventComponent;

	/** 시뮬레이션 상태 관리 컴포넌트 (배치 큐 + 결과 저장) */
	UPROPERTY(VisibleAnywhere, Category = "Hkt|Components")
	TObjectPtr<UHktSimulationStashComponent> SimulationStashComponent;

	/** Player Handle (Simulation과 연결) */
	UPROPERTY(Replicated)
	FHktPlayerHandle PlayerHandle;
};
