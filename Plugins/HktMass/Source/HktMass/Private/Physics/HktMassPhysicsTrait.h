// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassEntityTraitBase.h"
#include "HktMassPhysicsFragments.h"
#include "HktMassPhysicsTrait.generated.h"

UCLASS(MinimalAPI)
class UHktMassPhysicsTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Physics")
	FHktMassPhysicsParameters PhysicsParams;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDebugVisualization = false;

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
