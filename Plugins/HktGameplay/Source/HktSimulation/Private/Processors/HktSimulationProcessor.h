// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/HktJobQueue.h"
#include "Core/HktWaitQueue.h"
#include "Core/HktStateStore.h"
#include "Core/HktVMBatch.h"

// ============================================================================
// HktSimulation Processors - 기능별 분리된 처리 유닛
// 
// 설계 철학:
// - 단일 책임: 각 Processor는 하나의 역할만 담당
// - 명확한 인터페이스: Input/Output이 명확함
// - 테스트 용이: 독립적으로 테스트 가능
// ============================================================================

// 전방 선언
class UHktSimulationSubsystem;
struct FHktSimulationContext;

/**
 * 시뮬레이션 처리 컨텍스트
 * 
 * 모든 Processor가 공유하는 상태
 */
struct FHktSimulationContext
{
    // 큐
    FHktJobQueue* JobQueue = nullptr;
    FHktWaitQueue* WaitQueue = nullptr;
    
    // 상태 저장소
    FHktStateStore* StateStore = nullptr;
    FHktListStorage* ListStorage = nullptr;
    
    // 언리얼 월드
    UWorld* UnrealWorld = nullptr;
    
    // 틱 정보
    float DeltaTime = 0.0f;
    int32 FrameNumber = 0;
    
    // 이벤트 콜리전 (외부에서 설정)
    int32 LastCollisionEntityID = INDEX_NONE;
    int32 LastCollisionGeneration = 0;
    
    // 디버그
    bool bDebugLog = false;
    
    // 완료 이벤트 수집 (Cleanup에서 사용)
    TArray<int32> CompletedEventIDs;
    
    bool IsValid() const
    {
        return JobQueue && WaitQueue && StateStore && ListStorage;
    }
};

/**
 * 대기 완료 콜백 데이터
 * 
 * WaitProcessor에서 조건 충족 시 수집
 */
struct FHktWaitCompletion
{
    int32 RuntimeSlot = INDEX_NONE;
    int32 RuntimeGeneration = 0;
    int32 WaitQueueSlot = INDEX_NONE;
    EHktYieldCondition Condition = EHktYieldCondition::None;
    
    // 충돌 결과 (Collision 대기 완료 시)
    int32 CollisionEntityID = INDEX_NONE;
    int32 CollisionGeneration = 0;
};
