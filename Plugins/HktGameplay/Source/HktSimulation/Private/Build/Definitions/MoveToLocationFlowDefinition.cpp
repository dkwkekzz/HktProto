// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "Build/Definitions/MoveToLocationFlowDefinition.h"
#include "Build/FlowDefinitionRegistry.h"
#include "Core/HktVMBuilder.h"
#include "Core/HktAttributeStore.h"
#include "HktIntentInterface.h"
#include "NativeGameplayTags.h"

// 태그 정의
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Event_Action_Move, "Hkt.Event.Action.Move");

// 자동 등록
REGISTER_FLOW_DEFINITION(FMoveToLocationFlowDefinition)

FGameplayTag FMoveToLocationFlowDefinition::GetEventTag() const
{
    return TAG_Event_Action_Move;
}

bool FMoveToLocationFlowDefinition::BuildBytecode(
    FHktVMBuilder& Builder, 
    const FHktIntentEvent& Event, 
    FHktAttributeStore* Attributes)
{
    // ========================================================================
    // 위치 이동 Flow
    // 
    // 자연어로 읽으면:
    // "목표 위치로 이동하고, 도착하면 정지한다."
    // ========================================================================
    
    // "목표 위치로 이동하고"
    Builder.MoveTo(Event.Location);
    
    // "도착할 때까지 기다린다"
    Builder.WaitUntilArrival(10.0f);
    
    // "정지한다"
    Builder.Stop();
    
    // "끝"
    Builder.End();
    
    return true;
}

bool FMoveToLocationFlowDefinition::ValidateEvent(const FHktIntentEvent& Event) const
{
    // 이동 이벤트는 유효한 위치가 필요
    return Event.Location != FVector::ZeroVector;
}
