// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassNpcAITrait.h"
#include "HktMassNpcFragments.h"
#include "MassEntityTemplateRegistry.h"

void UHktMassNpcAITrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	// State Fragment (AI ?�태)
	FHktNpcStateFragment& StateFragment = BuildContext.AddFragment_GetRef<FHktNpcStateFragment>();
	StateFragment.CurrentState = 0; // Idle�??�작

	// Type Fragment (NPC ?�???�보)
	FHktNpcTypeFragment& TypeFragment = BuildContext.AddFragment_GetRef<FHktNpcTypeFragment>();
	TypeFragment.NpcType = NpcType;
	TypeFragment.TeamId = TeamId;
	TypeFragment.Level = Level;

	// Patrol Fragment (?�찰 경로)
	FHktNpcPatrolFragment& PatrolFragment = BuildContext.AddFragment_GetRef<FHktNpcPatrolFragment>();
	PatrolFragment.PatrolRadius = PatrolRadius;
	PatrolFragment.WaitTimeAtPoint = WaitTimeAtPoint;
}

