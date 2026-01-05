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
	virtual void DefineFlow(FHktFlowBuilder& FlowBuilder, const FHktIntentEvent& Event) override;

	/** 이 Behavior가 처리하는 이벤트 태그 */
	static FGameplayTag GetStaticEventTag();

	// === Configurable Parameters ===

	/** 파이어볼 비행 속도 */
	UPROPERTY(EditDefaultsOnly, Category = "Fireball")
	float FireballSpeed = 1500.0f;

	/** 폭발 범위 */
	UPROPERTY(EditDefaultsOnly, Category = "Fireball")
	float ExplosionRadius = 500.0f;

	/** 직격 데미지 */
	UPROPERTY(EditDefaultsOnly, Category = "Fireball")
	float DirectDamage = 100.0f;

	/** 폭발 범위 데미지 */
	UPROPERTY(EditDefaultsOnly, Category = "Fireball")
	float SplashDamage = 50.0f;

	/** 화염 지속 데미지 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Fireball")
	float BurningDuration = 10.0f;
};

/**
 * [Fireball Event Data]
 * 파이어볼 시전 이벤트에 포함된 데이터
 */
USTRUCT(BlueprintType)
struct HKTFLOW_API FHktFireballEventData
{
	GENERATED_BODY()

	/** 시전자 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 CasterHandle = INDEX_NONE;

	/** 시전 위치 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector CastLocation = FVector::ZeroVector;

	/** 발사 방향 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector Direction = FVector::ForwardVector;
};

