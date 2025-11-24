// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassPhysicsTrait.h"
#include "HktMassPhysicsFragments.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "Engine/World.h"

void UHktMassPhysicsTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.RequireFragment<FTransformFragment>();

	BuildContext.AddFragment<FHktMassVelocityFragment>();
	BuildContext.AddFragment<FHktMassForceFragment>();

	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	FConstSharedStruct LODParamsFragment = EntityManager.GetOrCreateConstSharedFragment(PhysicsParams);
	BuildContext.AddConstSharedFragment(LODParamsFragment);

	if (bDebugVisualization)
	{
		BuildContext.AddTag<FHktMassPhysicsDebugVisualizationTag>();
	}
}
