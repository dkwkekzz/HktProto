// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HktServiceInterface.h"
#include "HktSimulationInterface.generated.h"

// ---------------------------------------------------
// IHktSimulationProvider
// ---------------------------------------------------

UINTERFACE(MinimalAPI, Blueprintable)
class UHktSimulationProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 시뮬레이션 처리를 위임하기 위한 인터페이스
 * 이 인터페이스를 구현하는 객체(예: PlayerState)는 Intent Batch를 처리하고 결과를 반환해야 합니다.
 */
class HKTSIMULATION_API IHktSimulationProvider
{
	GENERATED_BODY()

public:
	/**
	 * Intent Event Batch를 처리하여 시뮬레이션 결과를 반환합니다.
	 * @param IntentBatch 처리할 이벤트 배치
	 * @return 시뮬레이션 결과
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hkt|Simulation")
	FHktSimulationResult ProcessSimulation(const FHktIntentEventBatch& IntentBatch);
};
