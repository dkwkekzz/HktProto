
// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassNpcTrait.h"
#include "HktMassNpcFragments.h"
#include "HktMassNpcAnimationTypes.h"
#include "MassEntityTemplateRegistry.h"
#include "MassRepresentationFragments.h"
#include "MassRepresentationSubsystem.h"
#include "MassLODFragments.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassVisualizationComponent.h"
#include "AnimToTextureDataAsset.h"

void UHktMassNpcTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	// Transform Fragment 추가
	BuildContext.AddFragment<FTransformFragment>();
	BuildContext.AddFragment<FMassVelocityFragment>();

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

	// Animation Fragment 추가
	if (AnimToTextureData)
	{
		FHktMassNpcAnimationFragment& AnimationFragment = BuildContext.AddFragment_GetRef<FHktMassNpcAnimationFragment>();
		AnimationFragment.AnimToTextureData = AnimToTextureData;
		UE_LOG(LogTemp, Warning, TEXT("HktMassNpcTrait: Added FHktMassNpcAnimationFragment with AnimToTextureData"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HktMassNpcTrait: No AnimToTextureData - FHktMassNpcAnimationFragment NOT added!"));
	}

	// UMassRepresentationSubsystem을 위한 Representation Fragment들 추가
	FMassRepresentationFragment& RepresentationFragment = BuildContext.AddFragment_GetRef<FMassRepresentationFragment>();
	UMassRepresentationSubsystem* RepresentationSubsystem = UWorld::GetSubsystem<UMassRepresentationSubsystem>(&World);
	if (RepresentationSubsystem)
	{
		RepresentationFragment.CurrentRepresentation = EMassRepresentationType::StaticMeshInstance;
		RepresentationFragment.StaticMeshDescHandle = RepresentationSubsystem->FindOrAddStaticMeshDesc(NpcMeshDesc);
	}

	BuildContext.AddFragment<FMassRepresentationLODFragment>();
	
	// ⭐ 중요! Visualization을 위한 Chunk Fragment 추가
	// 이게 없으면 UMassUpdateISMProcessor의 ChunkFilter를 통과하지 못해 Execute가 호출되지 않음!
	BuildContext.AddChunkFragment<FMassVisualizationChunkFragment>();

	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	FMassRepresentationSubsystemSharedFragment RepresentationSharedFragment;
	RepresentationSharedFragment.RepresentationSubsystem = RepresentationSubsystem;
	FSharedStruct SubsystemFragment = EntityManager.GetOrCreateSharedFragment(RepresentationSharedFragment);
	BuildContext.AddSharedFragment(SubsystemFragment);
}

