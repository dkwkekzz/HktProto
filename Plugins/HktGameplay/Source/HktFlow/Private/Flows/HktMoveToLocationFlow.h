// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HktFlowInterfaces.h"
#include "HktMoveToLocationBehavior.generated.h"

/**
 * [Move To Location Behavior]
 * 
 * 트리거: 클라이언트 이동키 입력 / 이동 이벤트
 * 
 * 흐름:
 * 1. 목표 위치를 향해 이동 시작
 * 2. 도착하면 → 이동 완료
 * 3. 충돌하면 → 질량에 따라 다른 방향으로 힘을 받음
 */
UCLASS()
class HKTFLOW_API UHktMoveToLocationBehavior : public UObject, public IHktFlow
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
 * [Move To Location Event Data]
 * 이동 이벤트에 포함된 데이터
 */
USTRUCT(BlueprintType)
struct HKTFLOW_API FHktMoveToLocationEventData
{
	GENERATED_BODY()

	/** 이동 주체 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 SubjectHandle = INDEX_NONE;

	/** 목표 위치 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector TargetLocation = FVector::ZeroVector;

	/** 이동 속도 배율 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float SpeedMultiplier = 1.0f;

	/** 주체의 질량 (충돌 시 힘 계산용) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Mass = 1.0f;
};

