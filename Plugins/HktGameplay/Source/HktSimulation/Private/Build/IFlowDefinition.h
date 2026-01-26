// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

// 전방 선언
class FHktVMBuilder;
struct FHktIntentEvent;

/**
 * [IFlowDefinition]
 * 
 * IntentEvent를 바이트코드로 변환하는 인터페이스
 * 각 Flow 타입은 이 인터페이스를 구현하여 동작을 정의
 * 
 * 설계 철학:
 * - Flow 정의는 자연어처럼 읽혀야 함
 * - 선형 흐름, 콜백 없음
 * - Builder의 Fluent API 사용
 * 
 * 예시:
 *   Builder.PlayAnimation(AnimTag)     // "애니메이션을 재생하고"
 *          .WaitSeconds(1.0f)          // "1초 기다린 뒤"
 *          .SpawnEntity(EntityTag, 0)  // "엔티티를 생성한다"
 *          .End();
 */
class HKTSIMULATION_API IFlowDefinition
{
public:
    virtual ~IFlowDefinition() = default;

    /**
     * 이벤트에 대한 바이트코드 빌드
     * 
     * @param Builder - VM 바이트코드 빌더 (Fluent API)
     * @param Event - 처리할 인텐트 이벤트
     * @return 성공 여부
     */
    virtual bool BuildBytecode(
        FHktVMBuilder& Builder, 
        const FHktIntentEvent& Event) = 0;

    /**
     * 이 Flow가 처리하는 GameplayTag 반환
     */
    virtual FGameplayTag GetEventTag() const = 0;

    /**
     * 우선순위 (높을수록 먼저 체크, 기본: 100)
     */
    virtual int32 GetPriority() const { return 100; }

    /**
     * 이벤트 유효성 검사 (빌드 전)
     */
    virtual bool ValidateEvent(const FHktIntentEvent& Event) const { return true; }
};
