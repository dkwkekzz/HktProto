// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassEntityTraitBase.h"
#include "Movement/HktMassMoveToTargetTrait.h"
#include "HktMassSimpleAITrait.generated.h"

//----------------------------------------------------------------------//
//  Fragments & Tags
//----------------------------------------------------------------------//

USTRUCT()
struct FHktMassPatrolFragment : public FMassFragment
{
	GENERATED_BODY()

	// 월드에서 수집한 경유지 좌표 목록
	UPROPERTY(VisibleAnywhere) // 에디터에서 디버깅용으로 보기만 가능
	TArray<FVector> Waypoints;

	int32 CurrentWaypointIndex = 0;

	UPROPERTY(EditAnywhere)
	float AcceptanceRadius = 100.0f;
};

USTRUCT()
struct FHktMassSimpleAITag : public FMassTag
{
	GENERATED_BODY()
};

//----------------------------------------------------------------------//
//  Traits
//----------------------------------------------------------------------//

/**
 * 지정된 태그를 가진 액터들의 위치를 찾아 순찰 경로로 설정하는 Trait
 */
UCLASS(MinimalAPI, meta = (DisplayName = "HKT Simple Patrol AI"))
class UHktMassSimpleAITrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	// 월드에 배치된 액터 중 이 태그를 가진 액터들을 찾아 순찰 지점으로 사용합니다.
	UPROPERTY(EditAnywhere, Category = "AI")
	FName PatrolActorTag;

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};