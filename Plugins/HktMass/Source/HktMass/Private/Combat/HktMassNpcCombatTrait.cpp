// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassNpcCombatTrait.h"
#include "HktMassNpcFragments.h"
#include "MassEntityTemplateRegistry.h"

void UHktMassNpcCombatTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	// Combat Fragment (체력, 공격????
	FHktNpcCombatFragment& CombatFragment = BuildContext.AddFragment_GetRef<FHktNpcCombatFragment>();
	CombatFragment.MaxHealth = MaxHealth;
	CombatFragment.CurrentHealth = MaxHealth;
	CombatFragment.AttackPower = AttackPower;
	CombatFragment.AttackRange = AttackRange;
	CombatFragment.AttackCooldown = AttackCooldown;

	// Target Fragment (?��??�보)
	BuildContext.AddFragment<FHktNpcTargetFragment>();
}

