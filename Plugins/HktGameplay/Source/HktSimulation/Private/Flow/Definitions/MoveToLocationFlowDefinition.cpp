// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "Flow/Definitions/MoveToLocationFlowDefinition.h"
#include "HktFlowBuilder.h"
#include "HktIntentInterface.h"
#include "HktEntityManager.h"
#include "Flow/FlowDefinitionRegistry.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Event_Action_Move, "Hkt.Event.Action.Move");

bool FMoveToLocationFlowDefinition::BuildBytecode(FHktFlowBuilder& Builder, const FHktIntentEvent& Event, FHktEntityManager* EntityManager)
{
	if (!EntityManager)
	{
		return false;
	}

	// 자연어: "목표 위치로 이동, 도착하면 정지"

	// 1. 목표로 이동 시작
	Builder.MoveTo(Event.Location);

	// 2. 도착까지 대기
	Builder.WaitUntilArrival(10.0f);

	// 3. 정지
	Builder.Stop();

	// 4. 종료
	Builder.End();

	return true;
}

FGameplayTag FMoveToLocationFlowDefinition::GetEventTag() const
{
	return TAG_Event_Action_Move;
}

bool FMoveToLocationFlowDefinition::ValidateEvent(const FHktIntentEvent& Event) const
{
	// Move events should have a valid location
	return Event.Location != FVector::ZeroVector;
}

// Auto-register this flow definition
// REGISTER_FLOW_DEFINITION(FMoveToLocationFlowDefinition)
