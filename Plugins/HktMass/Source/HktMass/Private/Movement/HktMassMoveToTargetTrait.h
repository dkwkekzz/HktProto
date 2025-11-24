// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassEntityTraitBase.h"
#include "HktMassMoveToTargetTrait.generated.h"

UCLASS(MinimalAPI, meta = (DisplayName = "Hkt Mass Move To Target"))
class UHktMassMoveToTargetTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
