// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "HktMassNpcFragments.generated.h"

// NPC??ì²´ë ¥ ë°??„íˆ¬ ?ì„±???´ëŠ” Fragment
USTRUCT()
struct FHktNpcCombatFragment : public FMassFragment
{
	GENERATED_BODY()

	// ?„ì¬ ì²´ë ¥
	UPROPERTY()
	float CurrentHealth = 100.0f;

	// ìµœë? ì²´ë ¥
	UPROPERTY()
	float MaxHealth = 100.0f;

	// ê³µê²©??
	UPROPERTY()
	float AttackPower = 10.0f;

	// ê³µê²© ë²”ìœ„
	UPROPERTY()
	float AttackRange = 150.0f;

	// ê³µê²© ì¿¨ë‹¤??
	UPROPERTY()
	float AttackCooldown = 1.0f;

	// ë§ˆì?ë§?ê³µê²© ?œê°„
	UPROPERTY()
	float LastAttackTime = 0.0f;
};

// NPC???€ê²??•ë³´ë¥??´ëŠ” Fragment
USTRUCT()
struct FHktNpcTargetFragment : public FMassFragment
{
	GENERATED_BODY()

	// ?€ê²??”í‹°??(?¤ë¥¸ NPC???Œë ˆ?´ì–´)
	//UPROPERTY()
	//FMassEntityHandle TargetEntity;

	// ?€ê²??„ì¹˜ (ê³ ì •???„ì¹˜ë¡??´ë™??ê²½ìš°)
	UPROPERTY()
	FVector TargetLocation = FVector::ZeroVector;

	// ?€ê²?ê±°ë¦¬
	UPROPERTY()
	float DistanceToTarget = 0.0f;

	// ?€ê²Ÿì´ ? íš¨?œì? ?¬ë?
	UPROPERTY()
	bool bHasValidTarget = false;
};

// NPC??AI ?íƒœë¥??´ëŠ” Fragment
USTRUCT()
struct FHktNpcStateFragment : public FMassFragment
{
	GENERATED_BODY()

	// AI ?íƒœ
	UPROPERTY()
	uint8 CurrentState = 0; // 0: Idle, 1: Patrol, 2: Chase, 3: Attack, 4: Dead

	// ?íƒœ ?€?´ë¨¸
	UPROPERTY()
	float StateTimer = 0.0f;

	// ?œì°° ?¬ì¸???¸ë±??
	UPROPERTY()
	int32 PatrolPointIndex = 0;
};

// NPC ?€???•ë³´ë¥??´ëŠ” Fragment (Tagë¡??€ì²?ê°€?¥í•˜ì§€ë§??ˆì‹œë¡??¬í•¨)
USTRUCT()
struct FHktNpcTypeFragment : public FMassFragment
{
	GENERATED_BODY()

	// NPC ?€??(0: Melee, 1: Ranged, 2: Tank, 3: Support)
	UPROPERTY()
	uint8 NpcType = 0;

	// NPC ?€ ID
	UPROPERTY()
	int32 TeamId = 0;

	// NPC ?ˆë²¨
	UPROPERTY()
	int32 Level = 1;
};

// ?œì°° ê²½ë¡œ ?•ë³´ë¥??´ëŠ” Fragment
USTRUCT()
struct FHktNpcPatrolFragment : public FMassFragment
{
	GENERATED_BODY()

	// ?œì°° ?¬ì¸?¸ë“¤
	UPROPERTY()
	TArray<FVector> PatrolPoints;

	// ?œì°° ë°˜ê²½
	UPROPERTY()
	float PatrolRadius = 500.0f;

	// ê°??¬ì¸?¸ì—???€ê¸??œê°„
	UPROPERTY()
	float WaitTimeAtPoint = 2.0f;
};

