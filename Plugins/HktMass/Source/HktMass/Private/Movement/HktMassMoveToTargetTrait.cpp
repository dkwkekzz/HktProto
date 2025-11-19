// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassMoveToTargetTrait.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "Engine/World.h"

void UHktMassMoveToTargetTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FTransformFragment>();
	BuildContext.AddFragment<FHktMassVelocityFragment>();
	BuildContext.AddFragment<FHktMassMoveToLocationFragment>();
}
