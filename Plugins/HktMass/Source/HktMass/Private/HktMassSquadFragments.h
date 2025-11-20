// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "HktMassSquadFragments.generated.h"

/**
 * 엔티티가 소속된 분대(Squad)의 ID를 저장
 */
USTRUCT()
struct FHktMassSquadMemberFragment : public FMassFragment
{
	GENERATED_BODY()

	// 소속될 분대 ID (0이면 소속 없음)
	UPROPERTY(EditAnywhere)
	int32 SquadID = 0;

	// 분대장과의 상대적 오프셋 (겹침 방지용)
	UPROPERTY()
	FVector FormationOffset = FVector::ZeroVector;
};
