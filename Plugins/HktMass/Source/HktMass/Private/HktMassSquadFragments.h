// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassEntityConfigAsset.h"
#include "HktMassSquadFragments.generated.h"

/**
 * 분대(Squad) 자체를 나타내는 프래그먼트
 */
USTRUCT()
struct FHktMassSquadFragment : public FMassFragment
{
	GENERATED_BODY()

	// 분대원 생성을 위한 설정
	UPROPERTY(EditAnywhere, Category = "Squad")
	TObjectPtr<UMassEntityConfigAsset> MemberConfig;

	// 생성할 분대원 수
	UPROPERTY(EditAnywhere, Category = "Squad")
	int32 MemberCount = 0;

	// 현재 분대 상태 (예: 이동, 전투, 대기)
	UPROPERTY(EditAnywhere, Category = "Squad")
	uint8 SquadState = 0;

	// 분대 최대 반경
	UPROPERTY(EditAnywhere, Category = "Squad")
	float SquadMaxRadius = 0.f;

	// 관리 중인 분대원들의 Entity Handle 목록
	UPROPERTY(Transient)
	TArray<FMassEntityHandle> MemberEntities;
};

/**
 * 엔티티가 소속된 분대(Squad) 정보를 저장
 */
USTRUCT()
struct FHktMassSquadMemberFragment : public FMassFragment
{
	GENERATED_BODY()

	// 소속된 분대 Entity Handle (직접 참조)
	UPROPERTY(Transient)
	FMassEntityHandle ParentSquadEntity;

	// 분대장(Squad Entity)과의 상대적 오프셋
	UPROPERTY()
	FVector FormationOffset = FVector::ZeroVector;

	// 분대장(Squad Entity)과의 최대 오프셋
	UPROPERTY()
	float MaxOffset = 0.f;
};

// 디버그 시각화용 Tag
USTRUCT()
struct FHktMassSquadDebugVisualizationTag : public FMassTag
{
    GENERATED_BODY()
};