// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "HktMassCommonFragments.generated.h"

// NPC??체력 �??�투 ?�성???�는 Fragment
USTRUCT()
struct FHktNpcCombatFragment : public FMassFragment
{
	GENERATED_BODY()

	// ?�재 체력
	UPROPERTY()
	float CurrentHealth = 100.0f;

	// 최�? 체력
	UPROPERTY()
	float MaxHealth = 100.0f;

	// 공격??
	UPROPERTY()
	float AttackPower = 10.0f;

	// 공격 범위
	UPROPERTY()
	float AttackRange = 150.0f;

	// 공격 쿨다??
	UPROPERTY()
	float AttackCooldown = 1.0f;

	// 마�?�?공격 ?�간
	UPROPERTY()
	float LastAttackTime = 0.0f;
};

// NPC???��??�보�??�는 Fragment
USTRUCT()
struct FHktNpcTargetFragment : public FMassFragment
{
	GENERATED_BODY()

	// ?��??�티??(?�른 NPC???�레?�어)
	//UPROPERTY()
	//FMassEntityHandle TargetEntity;

	// ?��??�치 (고정???�치�??�동??경우)
	UPROPERTY()
	FVector TargetLocation = FVector::ZeroVector;

	// ?��?거리
	UPROPERTY()
	float DistanceToTarget = 0.0f;

	// ?�겟이 ?�효?��? ?��?
	UPROPERTY()
	bool bHasValidTarget = false;
};

// NPC??AI ?�태�??�는 Fragment
USTRUCT()
struct FHktNpcStateFragment : public FMassFragment
{
	GENERATED_BODY()

	// AI ?�태
	UPROPERTY()
	uint8 CurrentState = 0; // 0: Idle, 1: Patrol, 2: Chase, 3: Attack, 4: Dead

	// ?�태 ?�?�머
	UPROPERTY()
	float StateTimer = 0.0f;

	// ?�찰 ?�인???�덱??
	UPROPERTY()
	int32 PatrolPointIndex = 0;
};

// NPC ?�???�보�??�는 Fragment (Tag�??��?가?�하지�??�시�??�함)
USTRUCT()
struct FHktNpcTypeFragment : public FMassFragment
{
	GENERATED_BODY()

	// NPC ?�??(0: Melee, 1: Ranged, 2: Tank, 3: Support)
	UPROPERTY()
	uint8 NpcType = 0;

	// NPC ?� ID
	UPROPERTY()
	int32 TeamId = 0;

	// NPC ?�벨
	UPROPERTY()
	int32 Level = 1;
};

// ?�찰 경로 ?�보�??�는 Fragment
USTRUCT()
struct FHktNpcPatrolFragment : public FMassFragment
{
	GENERATED_BODY()

	// ?�찰 ?�인?�들
	UPROPERTY()
	TArray<FVector> PatrolPoints;

	// ?�찰 반경
	UPROPERTY()
	float PatrolRadius = 500.0f;

	// �??�인?�에???��??�간
	UPROPERTY()
	float WaitTimeAtPoint = 2.0f;
};

