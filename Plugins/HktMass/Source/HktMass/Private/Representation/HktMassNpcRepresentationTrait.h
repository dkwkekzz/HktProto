// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassRepresentationTypes.h"
#include "HktMassNpcRepresentationTrait.generated.h"

/**
 * NPC ?úÍ∞Å??Í¥Ä??Trait
 * Representation FragmentÎ•?Ï∂îÍ??òÏó¨ ISM?ºÎ°ú ?åÎçîÎß?
 */
UCLASS(meta = (DisplayName = "Hkt Npc Representation"))
class HKTMASS_API UHktMassNpcRepresentationTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	// Static Mesh ?∏Ïä§?¥Ïä§ ?úÍ∞Å???§Ï†ï
	UPROPERTY(EditAnywhere, Category = "Representation")
	FStaticMeshInstanceVisualizationDesc NpcMeshDesc;

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};

