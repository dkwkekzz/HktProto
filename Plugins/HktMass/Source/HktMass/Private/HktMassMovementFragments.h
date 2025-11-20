// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "HktMassMovementFragments.generated.h"

// 이동 위치 Fragment
USTRUCT()
struct FHktMassMoveToLocationFragment : public FMassFragment
{
	GENERATED_BODY()

	// 이동 위치
	UPROPERTY()
	FVector TargetLocation = FVector::ZeroVector;
};

// 이동 목표 위치 Fragment
USTRUCT()
struct FHktMassVelocityFragment : public FMassFragment
{
	GENERATED_BODY()

	// 이동 속도
	UPROPERTY()
	FVector Value = FVector::ZeroVector;
};

