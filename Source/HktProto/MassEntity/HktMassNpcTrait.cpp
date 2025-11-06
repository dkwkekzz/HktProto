// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassNpcTrait.h"
#include "HktMassNpcFragments.h"
#include "HktMassNpcVisualizationProcessor.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "Engine/World.h"

void UHktMassNpcTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	// Transform Fragment 추가
	BuildContext.AddFragment<FTransformFragment>();

	// Movement Fragment 추가
	FHktNpcMovementFragment& MovementFragment = BuildContext.AddFragment_GetRef<FHktNpcMovementFragment>();
	MovementFragment.MaxSpeed = MaxSpeed;
	MovementFragment.CurrentSpeed = MaxSpeed * 0.8f; // 기본 속도는 최대 속도의 80%

	// Combat Fragment 추가
	FHktNpcCombatFragment& CombatFragment = BuildContext.AddFragment_GetRef<FHktNpcCombatFragment>();
	CombatFragment.MaxHealth = MaxHealth;
	CombatFragment.CurrentHealth = MaxHealth;
	CombatFragment.AttackPower = AttackPower;
	CombatFragment.AttackRange = AttackRange;
	CombatFragment.AttackCooldown = AttackCooldown;

	// Target Fragment 추가
	BuildContext.AddFragment<FHktNpcTargetFragment>();

	// State Fragment 추가
	FHktNpcStateFragment& StateFragment = BuildContext.AddFragment_GetRef<FHktNpcStateFragment>();
	StateFragment.CurrentState = 0; // Idle로 시작

	// Type Fragment 추가
	FHktNpcTypeFragment& TypeFragment = BuildContext.AddFragment_GetRef<FHktNpcTypeFragment>();
	TypeFragment.NpcType = NpcType;
	TypeFragment.TeamId = TeamId;
	TypeFragment.Level = Level;

	// Patrol Fragment 추가
	FHktNpcPatrolFragment& PatrolFragment = BuildContext.AddFragment_GetRef<FHktNpcPatrolFragment>();
	PatrolFragment.PatrolRadius = PatrolRadius;
	PatrolFragment.WaitTimeAtPoint = WaitTimeAtPoint;

	// Visualization Fragment 추가
	FHktNpcVisualizationFragment& VisualizationFragment = BuildContext.AddFragment_GetRef<FHktNpcVisualizationFragment>();
	VisualizationFragment.MeshType = NpcType;
	VisualizationFragment.Scale = FVector(1.0f, 1.0f, 1.0f);
}

