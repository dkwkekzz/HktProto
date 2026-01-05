// Copyright (c) 2026 Hkt Games. All Rights Reserved.

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "HktFlowInterfaces.h"
#include "IHktJobProvider.h"
#include "HktFlowSubsystem.generated.h"

// 로깅 카테고리 선언
DECLARE_LOG_CATEGORY_EXTERN(LogHktFlow, Log, All);

/**
 * UHktFlowSubsystem
 * - World Tick에 맞춰 이벤트를 동기화하고 Flow의 정의 및 생명주기를 관리하는 서브시스템
 * - 변경된 IHktFlow 사양에 따라, 매 틱마다 Flow를 실행(Tick)하는 것이 아니라
 * 이벤트 발생 시 Flow를 생성 및 정의(Define)하고, 이벤트 종료 시 정리하는 역할을 담당합니다.
 */
UCLASS()
class HKTFLOW_API UHktFlowSubsystem : public UTickableWorldSubsystem, public IHktJobProvider
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // UTickableWorldSubsystem Interface
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    // End UTickableWorldSubsystem Interface

    /** 태그에 매칭되는 Flow 생성 델리게이트 등록 (초기화 단계에서 호출 권장) */
    using FFlowFactoryFunc = TFunction<TSharedPtr<IHktFlow>()>;
    void RegisterFlowFactory(FGameplayTag Tag, FFlowFactoryFunc FactoryFunc);

protected:
    /** * 이벤트를 동기화하고 Flow 상태를 업데이트합니다. 
     * (기존에는 Flow->TickFlow를 호출했으나, 이제는 이벤트 동기화 및 DefineFlow 호출이 주 역할입니다)
     */
    void UpdateFlows(float DeltaTime);

    /** 개별 이벤트에 대한 Flow 생성 및 정의(DefineFlow) 호출 */
    void HandleFlowAdded(const FHktIntentEvent& NewEvent);

    /** 개별 이벤트에 대한 Flow 제거 및 관련 리소스 정리 */
    void HandleFlowRemoved(const FHktIntentEvent& RemovedEvent);

private:
    /** 활성화된 Flow 인스턴스 (EventId -> Flow Instance) */
    TMap<int32, TSharedPtr<IHktFlow>> ActiveFlows;

    /** 태그별 Flow 팩토리 */
    TMap<FGameplayTag, FFlowFactoryFunc> FlowFactories;

    /** 임시 히스토리 버퍼 (메모리 할당 최소화) */
    TArray<FHktIntentHistoryEntry> TempHistory;
};