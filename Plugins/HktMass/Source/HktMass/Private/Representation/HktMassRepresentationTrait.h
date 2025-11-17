// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassRepresentationTypes.h"
#include "HktMassRepresentationTrait.generated.h"

/**
 * Representation Trait
 * Representation Fragment를 추가하여 Static Mesh Instance로 더미 오브젝트 생성
 */
UCLASS(meta = (DisplayName = "Hkt Representation"))
class HKTMASS_API UHktMassRepresentationTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	// Static Mesh Visualization Description
	UPROPERTY(EditAnywhere, Category = "Representation")
	FStaticMeshInstanceVisualizationDesc MeshDesc;

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};

