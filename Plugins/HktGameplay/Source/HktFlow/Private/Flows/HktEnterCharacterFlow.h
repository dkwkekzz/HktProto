// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HktFlowInterfaces.h"
#include "HktEnterCharacterBehavior.generated.h"

/**
 * [Enter Character Behavior]
 * 
 * 트리거: 클라이언트 로그인
 * 
 * 흐름:
 * 1. 캐릭터 엔티티 생성
 * 2. 장착 아이템 이벤트 생성
 * 3. 캐릭터 등장 연출 (선택적)
 */
UCLASS()
class HKTFLOW_API UHktEnterCharacterBehavior : public UObject, public IHktFlow
{
	GENERATED_BODY()

public:
	// IHktFlow Interface
	virtual FGameplayTag GetEventTag() const override;
	virtual void DefineFlow(FHktFlowBuilder& Flow, const void* EventData) override;

	/** 이 Behavior가 처리하는 이벤트 태그 */
	static FGameplayTag GetStaticEventTag();
};

/**
 * [Enter Character Event Data]
 * 로그인 이벤트에 포함된 데이터
 */
USTRUCT(BlueprintType)
struct HKTFLOW_API FHktEnterCharacterEventData
{
	GENERATED_BODY()

	/** 플레이어 ID */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 PlayerId = INDEX_NONE;

	/** 캐릭터 타입 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName CharacterType;

	/** 스폰 위치 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector SpawnLocation = FVector::ZeroVector;

	/** 장착 아이템 목록 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FName> EquippedItems;
};

