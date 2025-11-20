// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "HktMassAIFragments.generated.h"

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

