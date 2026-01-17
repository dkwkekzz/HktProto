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

    /** 
     * 전역 Flow 등록 (Static)
     * - 모듈 로드 시점이나 static 초기화 시점에 호출하여 Flow 인스턴스(CDO 개념)를 등록합니다.
     */
    static void RegisterFlowInstance(FGameplayTag Tag, TSharedPtr<IHktFlow> FlowInstance);

protected:
    /** * 이벤트를 동기화하고 Flow 상태를 업데이트합니다. */
    void UpdateFlows(float DeltaTime);

    /** 태그에 해당하는 Flow 인스턴스를 가져옵니다 (전역 레지스트리 조회) */
    TSharedPtr<IHktFlow> GetFlow(FGameplayTag Tag);

private:
    /** 
     * 전역적으로 등록된 Flow 인스턴스 저장소 (Meyers Singleton Pattern)
     * - Static Initialization Order Fiasco 방지를 위해 함수 내 정적 변수로 관리
     */
    static TMap<FGameplayTag, TSharedPtr<IHktFlow>>& GetGlobalFlowRegistry();
};