// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "HktMassNpcFragments.generated.h"

// NPC의 기본 이동 속성을 담는 Fragment
USTRUCT()
struct FHktNpcMovementFragment : public FMassFragment
{
	GENERATED_BODY()

	// 현재 이동 속도
	UPROPERTY()
	float CurrentSpeed = 200.0f;

	// 최대 이동 속도
	UPROPERTY()
	float MaxSpeed = 400.0f;

	// 목표 방향
	UPROPERTY()
	FVector TargetDirection = FVector::ZeroVector;
};

// NPC의 체력 및 전투 속성을 담는 Fragment
USTRUCT()
struct FHktNpcCombatFragment : public FMassFragment
{
	GENERATED_BODY()

	// 현재 체력
	UPROPERTY()
	float CurrentHealth = 100.0f;

	// 최대 체력
	UPROPERTY()
	float MaxHealth = 100.0f;

	// 공격력
	UPROPERTY()
	float AttackPower = 10.0f;

	// 공격 범위
	UPROPERTY()
	float AttackRange = 150.0f;

	// 공격 쿨다운
	UPROPERTY()
	float AttackCooldown = 1.0f;

	// 마지막 공격 시간
	UPROPERTY()
	float LastAttackTime = 0.0f;
};

// NPC의 타겟 정보를 담는 Fragment
USTRUCT()
struct FHktNpcTargetFragment : public FMassFragment
{
	GENERATED_BODY()

	// 타겟 엔티티 (다른 NPC나 플레이어)
	//UPROPERTY()
	//FMassEntityHandle TargetEntity;

	// 타겟 위치 (고정된 위치로 이동할 경우)
	UPROPERTY()
	FVector TargetLocation = FVector::ZeroVector;

	// 타겟 거리
	UPROPERTY()
	float DistanceToTarget = 0.0f;

	// 타겟이 유효한지 여부
	UPROPERTY()
	bool bHasValidTarget = false;
};

// NPC의 AI 상태를 담는 Fragment
USTRUCT()
struct FHktNpcStateFragment : public FMassFragment
{
	GENERATED_BODY()

	// AI 상태
	UPROPERTY()
	uint8 CurrentState = 0; // 0: Idle, 1: Patrol, 2: Chase, 3: Attack, 4: Dead

	// 상태 타이머
	UPROPERTY()
	float StateTimer = 0.0f;

	// 순찰 포인트 인덱스
	UPROPERTY()
	int32 PatrolPointIndex = 0;
};

// NPC 타입 정보를 담는 Fragment (Tag로 대체 가능하지만 예시로 포함)
USTRUCT()
struct FHktNpcTypeFragment : public FMassFragment
{
	GENERATED_BODY()

	// NPC 타입 (0: Melee, 1: Ranged, 2: Tank, 3: Support)
	UPROPERTY()
	uint8 NpcType = 0;

	// NPC 팀 ID
	UPROPERTY()
	int32 TeamId = 0;

	// NPC 레벨
	UPROPERTY()
	int32 Level = 1;
};

// 순찰 경로 정보를 담는 Fragment
USTRUCT()
struct FHktNpcPatrolFragment : public FMassFragment
{
	GENERATED_BODY()

	// 순찰 포인트들
	UPROPERTY()
	TArray<FVector> PatrolPoints;

	// 순찰 반경
	UPROPERTY()
	float PatrolRadius = 500.0f;

	// 각 포인트에서 대기 시간
	UPROPERTY()
	float WaitTimeAtPoint = 2.0f;
};

