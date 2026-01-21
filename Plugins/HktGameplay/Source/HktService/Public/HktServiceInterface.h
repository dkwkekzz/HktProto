#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "InstancedStruct.h"
#include "HktServiceInterfaces.generated.h"

// ---------------------------------------------------
// Common Types
// ---------------------------------------------------

/**
 * Handle to uniquely identify a unit (abstracted from Mass/Actor).
 */
USTRUCT(BlueprintType)
struct FHktUnitHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 Value = -1;

	bool IsValid() const { return Value != -1; }

	bool operator==(const FHktUnitHandle& Other) const
	{
		return Value == Other.Value;
	}
};

/**
 * Handle to uniquely identify a player.
 */
USTRUCT(BlueprintType)
struct FHktPlayerHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 Value = -1;

	bool IsValid() const { return Value != -1; }

	bool operator==(const FHktUnitHandle& Other) const
	{
		return Value == Other.Value;
	}
};

/**
 * Player Attribute Types
 * 
 * 플레이어와 유닛이 공통으로 사용하는 속성 타입입니다.
 */
UENUM(BlueprintType)
enum class EHktAttributeType : uint8
{
	Health = 0,
	MaxHealth,
	Mana,
	MaxMana,
	AttackPower,
	Defense,
	MoveSpeed,
	Count	UMETA(Hidden)
};

// ---------------------------------------------------
// IHktSimulationProvider
// ---------------------------------------------------

UINTERFACE(MinimalAPI, BlueprintType)
class UHktSimulationProvider : public UInterface
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
class HKTSERVICE_API IHktSimulationProvider
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
