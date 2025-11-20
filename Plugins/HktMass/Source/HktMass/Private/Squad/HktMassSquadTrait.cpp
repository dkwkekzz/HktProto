// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassSquadTrait.h"
#include "HktMassMovementFragments.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"

void UHktMassSquadTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	// 이동 시스템 의존성 추가
	BuildContext.RequireFragment<FHktMassMoveToLocationFragment>();
	BuildContext.RequireFragment<FHktMassVelocityFragment>();
	BuildContext.RequireFragment<FTransformFragment>();

	FHktMassSquadMemberFragment& SquadMemberFrag = BuildContext.AddFragment_GetRef<FHktMassSquadMemberFragment>();
	SquadMemberFrag.SquadID = SquadID;
}
