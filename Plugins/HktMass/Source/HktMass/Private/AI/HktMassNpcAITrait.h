// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "HktMassNpcAITrait.generated.h"

/**
 * NPC AI Í¥Ä??Trait
 * State Fragment?Ä Patrol FragmentÎ•?Ï∂îÍ?
 */
UCLASS(meta = (DisplayName = "Hkt Npc AI"))
class HKTMASS_API UHktMassNpcAITrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	// NPC ?Ä??(0: Melee, 1: Ranged, 2: Tank)
	UPROPERTY(EditAnywhere, Category = "AI")
	uint8 NpcType = 0;

	// ?Ä ID
	UPROPERTY(EditAnywhere, Category = "AI")
	int32 TeamId = 0;

	// ?àÎ≤®
	UPROPERTY(EditAnywhere, Category = "AI")
	int32 Level = 1;

	// ?úÏ∞∞ Î∞òÍ≤Ω
	UPROPERTY(EditAnywhere, Category = "Patrol")
	float PatrolRadius = 500.0f;

	// ?úÏ∞∞ ?¨Ïù∏???ÄÍ∏??úÍ∞Ñ
	UPROPERTY(EditAnywhere, Category = "Patrol")
	float WaitTimeAtPoint = 2.0f;

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};

