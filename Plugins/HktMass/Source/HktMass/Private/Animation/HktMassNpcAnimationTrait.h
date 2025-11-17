// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "HktMassNpcAnimationTrait.generated.h"

class UAnimToTextureDataAsset;

/**
 * NPC ? ë‹ˆë©”ì´??ê´€??Trait
 * Animation Fragmentë¥?ì¶”ê?
 */
UCLASS(meta = (DisplayName = "Hkt Npc Animation"))
class HKTMASS_API UHktMassNpcAnimationTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	// AnimToTexture ?°ì´???ì…‹
	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimToTextureDataAsset> AnimToTextureData;

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};

