// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassSquadMemberTrait.h"
#include "HktMassMovementFragments.h"
#include "HktMassPhysicsFragments.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"

void UHktMassSquadMemberTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	// 이동 시스템 의존성 추가
	BuildContext.RequireFragment<FHktMassMoveToLocationFragment>();
	BuildContext.RequireFragment<FHktMassVelocityFragment>();
	BuildContext.RequireFragment<FTransformFragment>();

	// 분대원 정보 프래그먼트 추가 (값은 런타임에 설정됨)
	BuildContext.AddFragment<FHktMassSquadMemberFragment>();
}

