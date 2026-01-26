// Copyright Hkt Studios, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Build/IFlowDefinition.h"

struct FHktIntentEvent;
class FHktVMBuilder;
struct FGameplayTag;

/**
 * [FEnterCharacterFlowDefinition]
 * 
 * 캐릭터 입장 Flow 정의
 * 
 * 자연어 설명:
 * "캐릭터를 생성하고 스폰 애니메이션을 재생한다.
 *  0.5초 후 장비를 생성하고 인트로 애니메이션을 재생한다."
 */
class HKTSIMULATION_API FEnterCharacterFlowDefinition final : public IFlowDefinition
{
public:
    virtual FGameplayTag GetEventTag() const override;
    virtual bool BuildBytecode(FHktVMBuilder& Builder, const FHktIntentEvent& Event) override;
};
