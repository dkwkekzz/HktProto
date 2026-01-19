// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HktFlowInterface.h"
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
	virtual void DefineFlow(FHktJobBuilder& Builder, const FHktIntentEvent& Event) override;
};
