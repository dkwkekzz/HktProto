#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HktIntentInterface.h"
#include "Core/HktVMTypes.h"
#include "Core/HktVMBatch.h"
#include "Core/HktVMDispatch.h"
#include "Core/HktAttributeStore.h"
#include "HktSimulationSubsystem.generated.h"

// 전방 선언
class FHktFlowBuildProcessor;

/**
 * [UHktSimulationSubsystem]
 * 
 * 자연어 시뮬레이션 VM 중앙 관리자
 * 
 * 설계 철학:
 * - 자연어의 시간-공간 연속성: Flow는 선형으로 실행, 콜백 없음
 * - DOD 최적화: SoA 레이아웃, 캐시 친화적 배치 처리
 * - 서버/클라 공용: 결정론적 실행
 * 
 * 구성:
 * - VMBatch: 활성 VM들의 SoA 배치
 * - BuildProcessor: Flow 빌드 처리 (별도 클래스로 분리)
 * - AttributeStore: 엔티티 영구 속성 저장소
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
    
    // ========================================================================
    // 외부 API
    // ========================================================================
    
    // 속성 접근
    FHktAttributeStore* GetAttributeStore() { return &AttributeStore; }
    const FHktAttributeStore* GetAttributeStore() const { return &AttributeStore; }
    
    // 빌드 프로세서 접근
    FHktFlowBuildProcessor* GetBuildProcessor() { return BuildProcessor.Get(); }
    
    // VM 수동 시작 (테스트용)
    int32 StartVM(const FGameplayTag& FlowTag, int32 OwnerEntityID, int32 OwnerGeneration);
    
    // 디버그
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDebugLog = false;

private:
    // ========================================================================
    // 내부 처리
    // ========================================================================
    
    void ProcessBuildVMs();
    void ProcessActiveVMs(float DeltaTime);
    void ProcessCleanupFinishedVMs();
    
private:
    // ========================================================================
    // VM 배치 (SoA)
    // ========================================================================
    
    FHktVMBatch VMBatch;
    
    // ========================================================================
    // 빌드 프로세서 (Flow -> Program 변환)
    // ========================================================================
    
    TUniquePtr<FHktFlowBuildProcessor> BuildProcessor;
    
    // ========================================================================
    // 영구 속성 저장소
    // ========================================================================
    
    FHktAttributeStore AttributeStore;
    
    // ========================================================================
    // 임시 리스트 저장소 (쿼리 결과)
    // ========================================================================
    
    FHktListStorage ListStorage;
    
    // ========================================================================
    // VM 월드 컨텍스트
    // ========================================================================
    
    FHktVMWorld VMWorld;
};
