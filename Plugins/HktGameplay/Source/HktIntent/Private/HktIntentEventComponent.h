// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HktService/Public/HktIntentInterface.h"
#include "HktSimulation/Public/HktSimulationProvider.h"
#include "HktIntentEventComponent.generated.h"

/**
 * 클라이언트의 행동 의도(Intent)를 서버와 동기화하고, Lockstep 시뮬레이션을 수행하는 컴포넌트
 *
 * 데이터 흐름 (Lockstep Model):
 * 1. [Client] `Server_NotifyIntent()` RPC를 호출하여 서버로 의도(Event)를 보냅니다.
 * 2. [Server] 수신한 의도를 `PendingIntentEvents` 배열에 저장합니다. (복제되지 않음)
 * 3. [Server] GameMode가 `NotifyIntentBatch()`를 호출하면:
 *    - `PendingIntentEvents`를 `FHktIntentEventBatch`로 묶습니다.
 *    - 서버에서 시뮬레이션을 실행하고 `HktSimulationModel`에 결과를 저장합니다.
 *    - `Client_NotifyIntentBatch()` RPC를 호출하여 클라이언트에게 배치를 전송합니다.
 * 4. [Client] 배치를 수신하면 동일한 시뮬레이션을 실행합니다. (Lockstep - 서버와 동일한 결과)
 * 5. [Late-Join] `ProcessingIntentEventBatch`와 `HktSimulationModel`은 COND_InitialOnly로 새 클라이언트에게만 동기화됩니다.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HKTINTENT_API UHktIntentEventComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHktIntentEventComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- Public API ---

	/** 
	 * [Client] 서버로 Intent Event를 전송합니다.
	 * 내부적으로 Server RPC를 호출합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Hkt|Intent")
	void NotifyIntent(const FHktIntentEvent& IntentEvent);

	// --- Server-side hooks (called by GameMode) ---

	/** 
	 * [Server] GameMode에서 호출. 수집된 Intent들로 배치를 만들어 클라이언트에 전송을 시작합니다.
	 * @param FrameNumber 현재 처리할 프레임 번호
	 */
	void NotifyIntentBatch(int32 FrameNumber);

	/** 서버에게만 시뮬레이션 결과를 알립니다다. 클라이언트에서는 호출하지 않습니다. */
	UFUNCTION(BlueprintCallable, Category = "Hkt|Intent")
	void Server_NotifyCompletedSimulation(const FHktSimulationResult& SimulationResult);

protected:
	// --- RPCs ---

	/** [Client->Server RPC] 서버로 Intent Event를 전송합니다. */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_NotifyIntent(FHktIntentEvent IntentEvent);

	/** [Server->Client] Intent Event 배치를 클라이언트로 전송합니다. 클라이언트는 동일한 시뮬레이션을 실행합니다. */
	UFUNCTION(Client, Reliable)
	void Client_NotifyIntentBatch(const FHktIntentEventBatch& IntentBatch);

	/** Late-join 클라이언트를 위한 RepNotify: 처리 중인 배치 동기화 */
	UFUNCTION()
	void OnRep_ProcessingIntentBatch();
	
	/** Late-join 클라이언트를 위한 RepNotify: 마지막 시뮬레이션 결과 동기화 */
	UFUNCTION()
	void OnRep_HktSimulationModel();

private:
	/** [Server-Only] 다음 배치에 포함하기 위해 수집 중인 의도들 (복제되지 않음) */
	TArray<FHktIntentEvent> PendingIntentEvents;
	
	/**
	 * [Replicated for Late-Join] 현재 클라이언트에서 처리 중인 배치.
	 * COND_InitialOnly로 최초 접속 클라이언트에게만 동기화됩니다.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_ProcessingIntentBatch)
	FHktIntentEventBatch ProcessingIntentEventBatch;

	/**
	 * [Replicated for Late-Join] 가장 마지막으로 완료된 시뮬레이션 결과 모델.
	 * COND_InitialOnly로 최초 접속 클라이언트에게만 동기화됩니다.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_HktSimulationModel)
	FHktSimulationResult HktSimulationModel;

	/** 시뮬레이션 처리를 담당하는 Provider (UHktServiceSubsystem에서 획득) */
	TScriptInterface<IHktSimulationProvider> SimulationProvider;
};
