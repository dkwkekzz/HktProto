#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HktIntentInterface.h"
#include "Core/HktVMTypes.h"
#include "Core/HktJobQueue.h"
#include "Core/HktWaitQueue.h"
#include "Core/HktStateStore.h"
#include "Core/HktVMBatch.h"
#include "Processors/HktSimulationProcessor.h"
#include "HktSimulationSubsystem.generated.h"

// 전방 선언
class UHktSimulationStashComponent;
class FHktFlowBuildProcessor;
struct FHktCompletedEvent;

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
    
    void RegisterSimulationStashComponent(UHktSimulationStashComponent* Component);
    void UnregisterSimulationStashComponent(UHktSimulationStashComponent* Component);

    // ========================================================================
    // 외부 API
    // ========================================================================
    
    // 상태 저장소 접근
    FHktStateStore* GetStateStore() { return &StateStore; }
    const FHktStateStore* GetStateStore() const { return &StateStore; }
    
    // 빌드 프로세서 접근
    FHktFlowBuildProcessor* GetBuildProcessor() { return BuildProcessor.Get(); }
    
    // JobQueue 접근 (테스트/디버그용)
    FHktJobQueue* GetJobQueue() { return &JobQueue; }
    const FHktJobQueue* GetJobQueue() const { return &JobQueue; }
    
    // WaitQueue 접근
    FHktWaitQueue* GetWaitQueue() { return &WaitQueue; }
    const FHktWaitQueue* GetWaitQueue() const { return &WaitQueue; }
    
    /**
     * 새 VM 시작
     * 
     * @param FlowTag Flow 태그
     * @param EventID 이벤트 고유 ID (외부에서 관리)
     * @param OwnerEntityID 소유자 엔티티 ID
     * @param OwnerGeneration 소유자 Generation
     * @return VM 핸들, 실패 시 Invalid
     */
    FHktVMHandle StartVM(const FGameplayTag& FlowTag, int32 EventID, int32 OwnerEntityID, int32 OwnerGeneration);
    
    /**
     * VM 취소
     */
    void CancelVM(const FHktVMHandle& Handle);
    
    /**
     * 이벤트 ID로 VM 핸들 조회
     */
    FHktVMHandle GetVMHandleByEventID(int32 EventID) const;
    
    /**
     * 외부 충돌 이벤트 주입 (WaitCollision용)
     */
    void NotifyCollision(int32 EntityID, int32 Generation);
    
    // 디버그
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDebugLog = false;

private:
    // ========================================================================
    // 내부 처리 (Processor 호출)
    // ========================================================================
    
    void ProcessBuildVMs();      // IntentEvent → VM 생성
    void ProcessWaitConditions(); // 대기 조건 체크 (SOA)
    void ProcessActiveVMs();      // 명령어 실행 (AOS)
    void ProcessCleanupFinished(); // 완료된 VM 정리
    void ProcessNotifyCompletions(); // 완료 이벤트 전파
    
private:
    // ========================================================================
    // 작업 큐 (AOS - 직관적인 런타임 관리)
    // ========================================================================
    
    FHktJobQueue JobQueue;
    
    // ========================================================================
    // 대기 큐 (SOA - 캐시 효율적인 조건 체크)
    // ========================================================================
    
    FHktWaitQueue WaitQueue;
    
    // ========================================================================
    // 시뮬레이션 컨텍스트 (Processor들이 공유)
    // ========================================================================
    
    FHktSimulationContext SimContext;
    
    // ========================================================================
    // 빌드 프로세서 (Flow -> Program 변환)
    // ========================================================================
    
    TUniquePtr<FHktFlowBuildProcessor> BuildProcessor;
    
    // ========================================================================
    // 통합 상태 저장소
    // ========================================================================
    
    FHktStateStore StateStore;
    
    // ========================================================================
    // 임시 리스트 저장소 (쿼리 결과)
    // ========================================================================
    
    FHktListStorage ListStorage;
    
    // ========================================================================
    // 등록된 SimulationStashComponent
    // ========================================================================
    
    UPROPERTY()
    TArray<TObjectPtr<UHktSimulationStashComponent>> SimulationStashComponents;
    
    // ========================================================================
    // 완료 이벤트 (다음 틱에 전파)
    // ========================================================================
    
    TArray<FHktCompletedEvent> PendingCompletedEvents;
    
    // ========================================================================
    // 프레임 정보
    // ========================================================================
    
    int32 CompletedFrameNumber = 0;
};
