// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HktServiceInterface.h"
#include "HktSimulationInterface.generated.h"

// ============================================================================
// IHktFlow Interface
// ============================================================================

struct FHktJobBuilder;
struct FHktIntentEvent;

/**
 * IHktFlow
 * - Flow의 흐름을 정의하는 고수준 인터페이스입니다.
 * - Start/Tick/Stop의 직접적인 실행이 아닌, FHktJobBuilder를 통해 로직의 흐름(Flow)을 기술(Define)합니다.
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
     * FHktJobBuilder를 사용하여 인간의 언어 흐름과 유사한 고수준 추상화로 로직을 기술합니다.
     * 예: Builder.Do(CheckCondition).Then(PlayAnimation).Wait(2.0f).Finish();
     */
    virtual void DefineFlow(FHktFlowBuilder& Flow, const FHktIntentEvent& Event) = 0;
};

/**
 * Flow 자동 등록을 위한 헬퍼 매크로
 * 사용법: cpp 파일 상단에 HKT_REGISTER_FLOW(MyTag, FMyFlowClass) 작성
 */
#define HKT_REGISTER_FLOW(Tag, FlowClass) \
    static struct FAutoRegisterFlow_##FlowClass \
    {
        FAutoRegisterFlow_##FlowClass() \
        {
            /* 람다를 통해 팩토리 등록 */ \
            UHktFlowSubsystem::RegisterFlowType(Tag, []() { return MakeShared<FlowClass>(); }); \
        }
    } AutoRegisterFlow_##FlowClass;

// Note: UHktFlowSubsystem이 Private 폴더에 있으므로, 이 매크로를 쓰는 곳에서는 
// UHktFlowSubsystem의 static 메서드에 접근할 수 있어야 합니다.
// 하지만 헤더가 Private이라 직접 인클루드가 불가능할 수 있습니다.
// 이를 해결하기 위해 Public API 함수를 하나 노출합니다.

class HKTFLOW_API FHktFlowRegistry
{
public:
    // Factory 대신 인스턴스를 직접 등록합니다.
    static void RegisterFlow(FGameplayTag Tag, TSharedPtr<IHktFlow> FlowInstance);
};

#undef HKT_REGISTER_FLOW
#define HKT_REGISTER_FLOW(TagName, FlowClass) \
    static struct FAutoRegister_##FlowClass \
    {
        FAutoRegister_##FlowClass() \
        {
            /* CDO처럼 동작하도록 정적 초기화 시점에 인스턴스를 생성하여 등록합니다 */ \
            FHktFlowRegistry::RegisterFlow(TagName, MakeShared<FlowClass>()); \
        }
    } AutoRegister_##FlowClass;
