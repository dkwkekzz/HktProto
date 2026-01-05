// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "HktServiceInterfaces.h"
#include "HktFlowInterfaces.generated.h"

// ============================================================================
// IHktFlow Interface
// ============================================================================

struct FHktFlowBuilder;
struct FHktIntentEvent;

/**
 * IHktFlow
 * - Flow의 흐름을 정의하는 고수준 인터페이스입니다.
 * - Start/Tick/Stop의 직접적인 실행이 아닌, FHktFlowBuilder를 통해 로직의 흐름(Flow)을 기술(Define)합니다.
 * - 정의된 흐름은 이후 JobSystem 등으로 전파되어 실제로 실행됩니다.
 */
class HKTFLOW_API IHktFlow
{
    // GENERATED_BODY() // UInterface나 UObject로 전환 시 주석 해제 및 UCLASS/UINTERFACE 매크로 필요

public:
    virtual ~IHktFlow() = default;

    /** 이 Flow가 처리하는 이벤트 태그를 반환합니다. */
    virtual FGameplayTag GetEventTag() const = 0;

    /** * Flow의 흐름을 정의합니다.
     * FHktFlowBuilder를 사용하여 인간의 언어 흐름과 유사한 고수준 추상화로 로직을 기술합니다.
     * 예: Flow.Do(CheckCondition).Then(PlayAnimation).Wait(2.0f).Finish();
     */
    virtual void DefineFlow(FHktFlowBuilder& Flow, const FHktIntentEvent& Event) = 0;
};