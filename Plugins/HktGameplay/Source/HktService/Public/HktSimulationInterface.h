// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HktServiceInterface.h"
#include "HktSimulationInterface.generated.h"

// ---------------------------------------------------
// IHktSimulationProvider
// ---------------------------------------------------

UINTERFACE(MinimalAPI, BlueprintType)
class UHktSimulator : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for Simulation services.
 * Implemented by HktSimulation module.
 * 
 * 역할:
 * - 플레이어 등록/해제
 * - 속성 스냅샷 제공 (Late Join용)
 */
class HKTSERVICE_API IHktSimulator
{
	GENERATED_BODY()

public:
	/** 새 플레이어 등록 */
	virtual FHktPlayerHandle RegisterPlayer() = 0;
	
	/** 플레이어 등록 해제 */
	virtual void UnregisterPlayer(const FHktPlayerHandle& Handle) = 0;
	
	/** 플레이어 속성 스냅샷 조회 (Late Join용) */
	virtual bool GetPlayerSnapshot(const FHktPlayerHandle& Handle, TArray<float>& OutValues) const = 0;
	
	/** 플레이어 속성 스냅샷으로 초기화 (클라이언트에서 호출) */
	virtual void InitializePlayerFromSnapshot(const FHktPlayerHandle& Handle, const TArray<float>& Values) = 0;
};
