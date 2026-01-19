// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "MoveFlowDefinition.h"
#include "HktFlowBuilder.h"
#include "HktIntentInterface.h"
#include "HktEntityManager.h"
#include "HktFlowVM.h"
#include "Flow/FlowDefinitionRegistry.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Event_MoveToLocation, "Hkt.Event.MoveToLocation");

bool FMoveFlowDefinition::BuildBytecode(FHktFlowBuilder& Builder, const FHktIntentEvent& Event, FHktEntityManager* EntityManager)
{
	if (!EntityManager)
	{
		return false;
	}

	// For move events, we just store the target location in register
	// The actual movement is handled by movement systems
	// Here we just update the logic position
	
	// Store target location in GPR[0]
	// The calling code will set this up

	// In a more complete implementation, you might:
	// - Validate the path
	// - Play movement animations
	// - Set movement speed
	// For now, this is a minimal implementation

	return true;
}

FGameplayTag FMoveFlowDefinition::GetEventTag() const
{
	return TAG_Event_MoveToLocation;
}

// Auto-register this flow definition
REGISTER_FLOW_DEFINITION(FMoveFlowDefinition)
