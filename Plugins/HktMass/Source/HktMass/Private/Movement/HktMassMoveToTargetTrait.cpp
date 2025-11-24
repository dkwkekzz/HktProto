// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassMoveToTargetTrait.h"
#include "HktMassMovementFragments.h"
#include "HktMassPhysicsFragments.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "Engine/World.h"

void UHktMassMoveToTargetTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.RequireFragment<FTransformFragment>();
	BuildContext.AddFragment<FHktMassMoveToLocationFragment>();
}
