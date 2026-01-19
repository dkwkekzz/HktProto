// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HktServiceInterface.h"
#include "IHktPlayerAttributeProvider.generated.h"

/**
 * Player Attribute Types
 * 
 * 플레이어와 유닛이 공통으로 사용하는 속성 타입입니다.
 * HktSimulation 내부의 EHktAttribute와 동기화되어야 합니다.
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

/**
 * [Data Packet]
 * 단일 속성 변경을 나타내는 구조체입니다.
 * FFastArraySerializer의 Item으로 사용됩니다.
 */
USTRUCT(BlueprintType)
struct HKTSERVICE_API FHktAttributeEntry
{
	GENERATED_BODY()

	FHktAttributeEntry() = default;
	
	FHktAttributeEntry(EHktAttributeType InType, float InValue)
		: AttributeType(InType)
		, Value(InValue)
	{}

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	EHktAttributeType AttributeType = EHktAttributeType::Health;

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	float Value = 0.0f;
	
	bool operator==(const FHktAttributeEntry& Other) const
	{
		return AttributeType == Other.AttributeType;
	}
};

/**
 * [Batch Update]
 * 플레이어의 전체 속성 스냅샷입니다.
 * Provider에서 Consumer로 전달되는 데이터 패킷입니다.
 */
USTRUCT(BlueprintType)
struct HKTSERVICE_API FHktPlayerAttributeSnapshot
{
	GENERATED_BODY()

	/** 플레이어 핸들 (HktServiceInterface의 FHktPlayerHandle 사용) */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FHktPlayerHandle PlayerHandle;

	/** 변경된 속성 목록 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	TArray<FHktAttributeEntry> ChangedAttributes;

	/** 전체 속성 배열 (Index = EHktAttributeType) */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	TArray<float> AllAttributes;

	void InitializeAllAttributes()
	{
		AllAttributes.SetNumZeroed(static_cast<int32>(EHktAttributeType::Count));
		// 기본값 설정
		AllAttributes[static_cast<int32>(EHktAttributeType::MaxHealth)] = 100.0f;
		AllAttributes[static_cast<int32>(EHktAttributeType::Health)] = 100.0f;
		AllAttributes[static_cast<int32>(EHktAttributeType::MoveSpeed)] = 600.0f;
	}

	float GetAttribute(EHktAttributeType Type) const
	{
		const int32 Index = static_cast<int32>(Type);
		return AllAttributes.IsValidIndex(Index) ? AllAttributes[Index] : 0.0f;
	}

	void SetAttribute(EHktAttributeType Type, float NewValue)
	{
		const int32 Index = static_cast<int32>(Type);
		if (AllAttributes.IsValidIndex(Index))
		{
			AllAttributes[Index] = NewValue;
		}
	}
};

/**
 * Delegate for attribute change notifications
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerAttributeChanged, const FHktPlayerAttributeSnapshot& /* Snapshot */);

/**
 * Interface for providing player attribute updates from Simulation to Intent layer.
 * 
 * HktSimulation 내부에서 플레이어 속성이 변경되면,
 * 이 인터페이스를 통해 외부(HktIntent)에 전파합니다.
 * 
 * 구현: HktSimulationSubsystem
 * 소비: HktIntentSubsystem -> UHktAttributeComponent
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UHktPlayerAttributeProvider : public UInterface
{
	GENERATED_BODY()
};

class HKTSERVICE_API IHktPlayerAttributeProvider
{
	GENERATED_BODY()

public:
	/**
	 * 속성 변경 델리게이트 바인딩
	 * Consumer(HktIntent)가 변경 알림을 받기 위해 바인딩합니다.
	 */
	virtual FOnPlayerAttributeChanged& OnPlayerAttributeChanged() = 0;

	/**
	 * 특정 플레이어의 현재 속성 스냅샷을 조회합니다.
	 * @param PlayerHandle 조회할 플레이어
	 * @param OutSnapshot 출력 스냅샷
	 * @return 성공 여부
	 */
	virtual bool GetPlayerAttributeSnapshot(const FHktPlayerHandle& PlayerHandle, FHktPlayerAttributeSnapshot& OutSnapshot) const = 0;

	/**
	 * 모든 Dirty 플레이어의 스냅샷을 조회하고 Dirty 플래그를 클리어합니다.
	 * @param OutSnapshots Dirty 플레이어들의 스냅샷 목록
	 * @return Dirty 플레이어가 있으면 true
	 */
	virtual bool ConsumeChangedPlayers(TArray<FHktPlayerAttributeSnapshot>& OutSnapshots) = 0;

	/**
	 * 새 플레이어 등록 시 호출
	 * @return 새 플레이어의 핸들
	 */
	virtual FHktPlayerHandle RegisterPlayer() = 0;

	/**
	 * 플레이어 등록 해제
	 */
	virtual void UnregisterPlayer(const FHktPlayerHandle& PlayerHandle) = 0;
};
