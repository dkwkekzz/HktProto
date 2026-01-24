// Copyright Hkt Studios, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Build/IFlowDefinition.h"

struct FHktIntentEvent;
class FHktVMBuilder;
struct FHktAttributeStore;
struct FGameplayTag;

/**
 * [FFireballFlowDefinition]
 * 
 * 파이어볼 스킬 Flow 정의
 * 
 * 자연어 설명:
 * "시전 애니메이션을 재생하고 1초 기다린다.
 *  파이어볼을 생성하여 앞으로 날린다.
 *  충돌하면 파이어볼을 제거하고 직격 대상에게 100 피해를 준다.
 *  주변 300 범위 내 대상들에게 각각 50 피해와 화상을 입힌다."
 */
class HKTSIMULATION_API FFireballFlowDefinition final : public IFlowDefinition
{
public:
    virtual FGameplayTag GetEventTag() const override;
    virtual bool BuildBytecode(FHktVMBuilder& Builder, const FHktIntentEvent& Event, FHktAttributeStore* Attributes) override;
};
