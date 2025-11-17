// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "HktMassNpcCombatTrait.generated.h"

/**
 * NPC ?�투 관??Trait
 * Combat Fragment?� Target Fragment�?추�?
 */
UCLASS(meta = (DisplayName = "Hkt Npc Combat"))
class HKTMASS_API UHktMassNpcCombatTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	// 최�? 체력
	UPROPERTY(EditAnywhere, Category = "Combat")
	float MaxHealth = 100.0f;

	// 공격??
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackPower = 10.0f;

	// 공격 범위
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackRange = 150.0f;

	// 공격 쿨다??
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackCooldown = 1.0f;

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};

