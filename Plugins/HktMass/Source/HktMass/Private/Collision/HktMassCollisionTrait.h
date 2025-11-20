// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "HktMassCollisionFragments.h"
#include "HktMassCollisionTrait.generated.h"

UCLASS(meta = (DisplayName = "Hkt Simple Collision"))
class UHktMassCollisionTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	UPROPERTY(EditAnywhere, Category = "Mass|Collision")
	FHktMassCollisionFragment Collision;
};

