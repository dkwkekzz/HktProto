// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "HktMassCollisionFragments.generated.h"

// 충돌 반지름을 정의하는 프래그먼트
USTRUCT()
struct FHktMassCollisionFragment : public FMassFragment
{
	GENERATED_BODY()

	// 충돌 범위 반지름 (cm)
	UPROPERTY(EditAnywhere, Category = "Mass|Collision")
	float Radius = 40.0f;

	// 충돌 시 밀려나는 힘의 계수
	UPROPERTY(EditAnywhere, Category = "Mass|Collision")
	float RepulsionForce = 500.0f;
};

// 충돌이 발생했음을 알리는 태그 (이벤트 대신 사용)
// 다음 프레임에 StateTree나 다른 프로세서에서 이 태그를 감지하여 로직(예: 데미지, 소리)을 수행
USTRUCT()
struct FHktMassCollisionHitTag : public FMassTag
{
	GENERATED_BODY()
};

