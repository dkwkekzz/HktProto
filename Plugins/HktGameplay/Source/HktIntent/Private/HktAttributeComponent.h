// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "HktServiceInterface.h"
#include "HktAttributeComponent.generated.h"

class UHktAttributeComponent;

/**
 * [Data Packet]
 * 단일 속성을 FFastArraySerializer로 리플리케이션하기 위한 Item
 */
USTRUCT(BlueprintType)
struct FHktAttributeItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FHktAttributeItem() = default;
	
	FHktAttributeItem(EHktAttributeType InType, float InValue)
		: AttributeType(InType)
		, Value(InValue)
	{}

	/** 속성 타입 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	EHktAttributeType AttributeType = EHktAttributeType::Health;

	/** 속성 값 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	float Value = 0.0f;

	// FFastArraySerializerItem Callbacks
	void PostReplicatedAdd(const struct FHktAttributeContainer& InArraySerializer);
	void PostReplicatedChange(const struct FHktAttributeContainer& InArraySerializer);
	void PreReplicatedRemove(const struct FHktAttributeContainer& InArraySerializer);
};

/**
 * [Data Container]
 * 플레이어 속성 리스트를 FFastArraySerializer로 관리
 * 델타 직렬화로 변경된 속성만 네트워크 전송
 */
USTRUCT(BlueprintType)
struct FHktAttributeContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FHktAttributeItem> Items;

	/** 소유 컴포넌트 참조 (콜백에서 사용) */
	UPROPERTY(NotReplicated)
	TObjectPtr<UHktAttributeComponent> OwnerComponent = nullptr;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FHktAttributeItem, FHktAttributeContainer>(Items, DeltaParms, *this);
	}

	// --- Helper Methods ---
	
	/** 속성 추가 또는 업데이트 */
	void SetAttribute(EHktAttributeType Type, float NewValue);
	
	/** 모든 속성 일괄 설정 */
	void SetAllAttributes(const TArray<float>& Values);
	
	/** 속성 조회 */
	float GetAttribute(EHktAttributeType Type) const;
	
	/** 모든 속성 초기화 */
	void InitializeDefaults();

private:
	/** 타입으로 Item 인덱스 찾기 */
	int32 FindItemIndex(EHktAttributeType Type) const;
};

// FFastArraySerializer Traits
template<>
struct TStructOpsTypeTraits<FHktAttributeContainer> : public TStructOpsTypeTraitsBase2<FHktAttributeContainer>
{
	enum { WithNetDeltaSerializer = true };
};

/**
 * Delegate for attribute change notifications (Client side)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeChangedDelegate, EHktAttributeType, AttributeType, float, NewValue);

/**
 * 플레이어 속성을 리플리케이션하는 컴포넌트
 * 
 * FFastArraySerializer를 사용하여 델타 직렬화로 효율적인 네트워크 동기화
 * HktIntentPlayerState에 부착되어 플레이어 속성을 관리
 * 
 * 데이터 흐름:
 * Server: Simulation -> ServiceSubsystem(Provider) -> IntentSubsystem -> Component(FAS)
 * Client: Component(FAS) -> OnRep callbacks -> UI
 */
UCLASS(ClassGroup=(Hkt), meta=(BlueprintSpawnableComponent))
class HKTINTENT_API UHktAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHktAttributeComponent();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- Server API (Simulation이 Sink를 통해 호출) ---
	
	/**
	 * 플레이어 핸들 설정 (Server only)
	 */
	void SetPlayerHandle(const FHktPlayerHandle& InHandle);
	
	/**
	 * 단일 속성 즉시 설정 (Server only)
	 * Simulation이 PushAttribute() 시 호출됨
	 */
	void SetAttribute(EHktAttributeType Type, float Value);
	
	/**
	 * 모든 속성 일괄 설정 (Server only)
	 * Simulation이 PushAllAttributes() 시 호출됨
	 */
	void SetAllAttributes(const TArray<float>& Values);

	// --- Accessors ---
	
	UFUNCTION(BlueprintPure, Category = "Hkt|Attributes")
	FHktPlayerHandle GetPlayerHandle() const { return PlayerHandle; }

	UFUNCTION(BlueprintPure, Category = "Hkt|Attributes")
	float GetHealth() const { return AttributeContainer.GetAttribute(EHktAttributeType::Health); }

	UFUNCTION(BlueprintPure, Category = "Hkt|Attributes")
	float GetMaxHealth() const { return AttributeContainer.GetAttribute(EHktAttributeType::MaxHealth); }

	UFUNCTION(BlueprintPure, Category = "Hkt|Attributes")
	float GetMana() const { return AttributeContainer.GetAttribute(EHktAttributeType::Mana); }

	UFUNCTION(BlueprintPure, Category = "Hkt|Attributes")
	float GetMaxMana() const { return AttributeContainer.GetAttribute(EHktAttributeType::MaxMana); }

	UFUNCTION(BlueprintPure, Category = "Hkt|Attributes")
	float GetAttackPower() const { return AttributeContainer.GetAttribute(EHktAttributeType::AttackPower); }

	UFUNCTION(BlueprintPure, Category = "Hkt|Attributes")
	float GetDefense() const { return AttributeContainer.GetAttribute(EHktAttributeType::Defense); }

	UFUNCTION(BlueprintPure, Category = "Hkt|Attributes")
	float GetMoveSpeed() const { return AttributeContainer.GetAttribute(EHktAttributeType::MoveSpeed); }

	UFUNCTION(BlueprintPure, Category = "Hkt|Attributes")
	float GetAttribute(EHktAttributeType Type) const { return AttributeContainer.GetAttribute(Type); }

	// --- Delegates ---
	
	/** 속성 변경 시 브로드캐스트 (Client에서 UI 업데이트용) */
	UPROPERTY(BlueprintAssignable, Category = "Hkt|Attributes")
	FOnAttributeChangedDelegate OnAttributeChanged;

	/** 내부 콜백 (FAS에서 호출) */
	void NotifyAttributeChanged(EHktAttributeType Type, float NewValue);

private:
	/** 플레이어 핸들 (Simulation과 연결) */
	UPROPERTY(Replicated)
	FHktPlayerHandle PlayerHandle;

	/** 속성 컨테이너 (FFastArraySerializer) */
	UPROPERTY(Replicated)
	FHktAttributeContainer AttributeContainer;
};
