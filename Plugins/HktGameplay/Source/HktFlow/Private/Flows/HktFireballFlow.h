// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HktFlowInterfaces.h"
#include "HktFireballBehavior.generated.h"

/**
 * [Fireball Behavior]
 * 
 * 트리거: 파이어볼 액션키 입력
 * 
 * 흐름:
 * 1. 파이어볼 시전 애니메이션 실행
 * 2. 파이어볼 엔티티 생성
 * 3. 파이어볼은 스스로 앞으로 나아감
 * 4. 충돌하면:
 *    - 파이어볼 폭발 및 파괴
 *    - 폭발은 주변 범위에 피해 전파
 *    - 피해를 받은 주체들은 10초간 화염 데미지 (Burning DOT)
 */
UCLASS()
class HKTFLOW_API UHktFireballBehavior : public UObject, public IHktFlow
{
	GENERATED_BODY()

public:
	// IHktFlow Interface
	virtual FGameplayTag GetEventTag() const override;
	virtual void DefineFlow(FHktJobBuilder& Builder, const FHktIntentEvent& Event) override;
};
