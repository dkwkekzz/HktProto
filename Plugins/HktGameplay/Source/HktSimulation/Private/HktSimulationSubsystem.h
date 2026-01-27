#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HktIntentInterface.h"
#include "Core/VMTypes.h"
#include "Core/VMProcessor.h"
#include "HktSimulationSubsystem.generated.h"

// 전방 선언
class UHktSimulationStashComponent;

USTRUCT()
struct HKTSIMULATION_API FHktSimulationContext
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<UHktSimulationStashComponent> StashComponent;

    UPROPERTY()
    TScriptInterface<IHktIntentEventProvider> IntentEventProvider;

    FVMProcessor VMProcessor;
};

/**
 * [UHktSimulationSubsystem]
 * 
 * 자연어 시뮬레이션 VM 중앙 관리자
 * 
 * 설계 철학:
 * - 자연어의 시간-공간 연속성: Flow는 선형으로 실행, 콜백 없음
 * - DOD 최적화: 대기 데이터는 SoA, 런타임은 AoS
 * - 서버/클라 공용: 결정론적 실행
 * 
 * 구성:
 * - JobQueue: 활성 VM 런타임 관리 (AoS)
 * - WaitQueue: 대기 중인 VM의 조건 데이터 (SoA)
 * - Processors: 빌드, 실행, 대기체크, 정리 분리
 * - StateStore: 엔티티/플레이어/프로세스 통합 상태 저장소
 * - ListStorage: 쿼리 결과 임시 저장소
 */
UCLASS()
class HKTSIMULATION_API UHktSimulationSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    // ========================================================================
    // 라이프사이클
    // ========================================================================
    
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    
    static UHktSimulationSubsystem* Get(const UObject* WorldContextObject);

    // ========================================================================
    // SimulationStashComponent 등록
    // ========================================================================
    
    void RegisterStashComponent(UHktSimulationStashComponent* Component);
    void UnregisterStashComponent(UHktSimulationStashComponent* Component);

private:
    void CreateSimulationContext(UHktSimulationStashComponent* Component);
    void DestroySimulationContext(UHktSimulationStashComponent* Component);

private:
    UPROPERTY()
    TArray<FHktSimulationContext> SimulationContexts;   
    
    UPROPERTY()
    int32 CompletedFrameNumber = 0;
};
