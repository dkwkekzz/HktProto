// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "Collision/HktMassCollisionTrait.h"
#include "HktMassCollisionFragments.h"
#include "MassEntityTemplateRegistry.h"

void UHktMassCollisionTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FHktMassCollisionFragment& CollisionFragment = BuildContext.AddFragment_GetRef<FHktMassCollisionFragment>();
	CollisionFragment = Collision;
}

