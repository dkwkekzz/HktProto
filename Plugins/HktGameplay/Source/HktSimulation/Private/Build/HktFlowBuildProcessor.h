// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Core/HktVMProgram.h"

// 전방 선언
struct FHktIntentEvent;
struct FHktAttributeStore;

/**
 * [FHktFlowBuildProcessor]
 * 
 * Flow 빌드 처리 전담 클래스
 * 
 * 역할:
 * - FlowDefinitionRegistry에서 Flow 정의 조회
 * - IFlowDefinition을 통해 FHktProgram 빌드
 * - 빌드된 프로그램 캐싱
 * 
 * HktSimulationSubsystem에서 분리되어 단일 책임 원칙 준수
 */
class HKTSIMULATION_API FHktFlowBuildProcessor
{
public:
    FHktFlowBuildProcessor();
    ~FHktFlowBuildProcessor();
    
    /**
     * 프로그램 가져오기 (캐시 우선, 없으면 빌드)
     * 
     * @param FlowTag - Flow를 식별하는 GameplayTag
     * @param Event - 빌드 컨텍스트를 제공하는 이벤트 (선택적)
     * @param Attributes - 속성 저장소 참조 (선택적)
     * @return 빌드된 프로그램, 실패 시 nullptr
     */
    const FHktProgram* GetOrBuildProgram(
        const FGameplayTag& FlowTag,
        const FHktIntentEvent* Event = nullptr,
        FHktAttributeStore* Attributes = nullptr);
    
    /**
     * 프로그램 강제 리빌드 (캐시 무시)
     */
    const FHktProgram* RebuildProgram(
        const FGameplayTag& FlowTag,
        const FHktIntentEvent* Event = nullptr,
        FHktAttributeStore* Attributes = nullptr);
    
    /**
     * 캐시에서 프로그램 가져오기 (빌드 없음)
     */
    const FHktProgram* GetCachedProgram(const FGameplayTag& FlowTag) const;
    
    /**
     * 특정 태그의 캐시 무효화
     */
    void InvalidateCache(const FGameplayTag& FlowTag);
    
    /**
     * 전체 캐시 클리어
     */
    void ClearCache();
    
    /**
     * 캐시 통계
     */
    int32 GetCachedProgramCount() const;
    
    /**
     * 디버그 로깅 설정
     */
    void SetDebugLog(bool bEnable) { bDebugLog = bEnable; }
    
private:
    /**
     * FlowDefinition을 찾아 프로그램 빌드
     */
    TSharedPtr<FHktProgram> BuildProgramFromDefinition(
        const FGameplayTag& FlowTag,
        const FHktIntentEvent* Event,
        FHktAttributeStore* Attributes);
    
private:
    // 프로그램 캐시
    FHktProgramCache ProgramCache;
    
    // 디버그 로깅
    bool bDebugLog = false;
};
