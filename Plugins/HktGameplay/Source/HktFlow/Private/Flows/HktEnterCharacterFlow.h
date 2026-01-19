// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HktFlowInterface.h"
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
	virtual void DefineFlow(FHktJobBuilder& Builder, const FHktIntentEvent& Event) override;
};

