// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "SkillFlowDefinition.h"
#include "HktFlowBuilder.h"
#include "HktIntentInterface.h"
#include "HktEntityManager.h"
#include "HktFlowVM.h"
#include "Flow/FlowDefinitionRegistry.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Event_Ability, "Hkt.Event.Ability");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Event_Ability_Fireball, "Hkt.Event.Ability.Fireball");

bool FSkillFlowDefinition::BuildBytecode(FHktFlowBuilder& Builder, const FHktIntentEvent& Event, FHktEntityManager* EntityManager)
{
	if (!EntityManager)
	{
		return false;
	}

	// Check for specific skill types
	FString TagString = Event.EventTag.ToString();
	
	if (Event.EventTag.MatchesTag(TAG_Event_Ability_Fireball))
	{
		return BuildFireballSkill(Builder, Event);
	}

	// Generic skill handling
	// Play casting animation
	Builder.PlayAnim(FName("Montage_Cast"))
		.WaitSeconds(0.3f);

	return true;
}

bool FSkillFlowDefinition::BuildFireballSkill(FHktFlowBuilder& Builder, const FHktIntentEvent& Event)
{
	constexpr uint8 RegProjectile = 0;
	
	// Fireball sequence:
	// 1. Play cast animation
	// 2. Wait for cast time
	// 3. Spawn projectile (would need projectile class from asset system)
	// 4. Wait for impact
	// 5. Explode and damage

	Builder.PlayAnim(FName("Montage_Cast"))
		.WaitSeconds(0.3f);
	
	// Note: SpawnFireball requires a UClass* which we don't have here
	// In a complete implementation, you'd look this up from an asset registry
	// For now, we comment this out
	/*
	Builder.SpawnFireball(FireballClass, RegProjectile)
		.WaitForImpact(RegProjectile)
		.Explode(500.0f, 100.0f, 10.0f, 5.0f, RegProjectile);
	*/

	return true;
}

FGameplayTag FSkillFlowDefinition::GetEventTag() const
{
	return TAG_Event_Ability;
}

// Auto-register this flow definition
REGISTER_FLOW_DEFINITION(FSkillFlowDefinition)
