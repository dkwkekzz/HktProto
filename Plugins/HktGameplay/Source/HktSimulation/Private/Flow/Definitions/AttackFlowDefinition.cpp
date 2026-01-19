// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "AttackFlowDefinition.h"
#include "HktFlowBuilder.h"
#include "HktIntentInterface.h"
#include "HktEntityManager.h"
#include "HktFlowVM.h"
#include "Flow/FlowDefinitionRegistry.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Event_Action_Attack, "Hkt.Event.Action.Attack");

bool FAttackFlowDefinition::BuildBytecode(FHktFlowBuilder& Builder, const FHktIntentEvent& Event, FHktEntityManager* EntityManager)
{
	if (!EntityManager)
	{
		return false;
	}

	// Attack requires a valid target
	if (!Event.Target.IsValid())
	{
		return false;
	}

	// Get or create internal handle for target
	// Note: This is a simplified version - in production you'd pass a handle mapper
	// For now, we assume the calling code will handle this
	
	float Damage = Event.Magnitude > 0.0f ? Event.Magnitude : 10.0f;
	
	// Build bytecode: Apply damage to target
	// Target handle should be in GPR[1] (set by calling code)
	Builder.ModifyHealth(1, -Damage);

	return true;
}

FGameplayTag FAttackFlowDefinition::GetEventTag() const
{
	return TAG_Event_Action_Attack;
}

bool FAttackFlowDefinition::ValidateEvent(const FHktIntentEvent& Event) const
{
	// Attack events must have a valid target
	return Event.Target.IsValid();
}

// Auto-register this flow definition
REGISTER_FLOW_DEFINITION(FAttackFlowDefinition)
