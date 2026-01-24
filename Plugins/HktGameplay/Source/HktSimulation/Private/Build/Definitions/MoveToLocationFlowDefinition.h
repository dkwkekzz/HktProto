// Copyright Hkt Studios, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Build/IFlowDefinition.h"

struct FHktIntentEvent;
class FHktVMBuilder;
struct FHktAttributeStore;
struct FGameplayTag;

/**
 * [FMoveToLocationFlowDefinition]
 * 
 * 위치 이동 Flow 정의
 * 
 * 자연어 설명:
 * "목표 위치로 이동하고, 도착하면 정지한다."
 */
class HKTSIMULATION_API FMoveToLocationFlowDefinition final : public IFlowDefinition
{
public:
    virtual FGameplayTag GetEventTag() const override;
    virtual bool BuildBytecode(FHktVMBuilder& Builder, const FHktIntentEvent& Event, FHktAttributeStore* Attributes) override;
    virtual bool ValidateEvent(const FHktIntentEvent& Event) const override;
};
