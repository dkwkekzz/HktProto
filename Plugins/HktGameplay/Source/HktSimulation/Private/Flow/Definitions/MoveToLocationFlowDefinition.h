// Copyright Hkt Studios, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Flow/IFlowDefinition.h"

struct FHktIntentEvent;
class FHktFlowBuilder;
class FHktEntityManager;
struct FGameplayTag;

class HKTSIMULATION_API FMoveToLocationFlowDefinition final : public IFlowDefinition
{
public:
	virtual FGameplayTag GetEventTag() const override;
	virtual bool BuildBytecode(FHktFlowBuilder& Builder, const FHktIntentEvent& Event, FHktEntityManager* EntityManager) override;
	virtual bool ValidateEvent(const FHktIntentEvent& Event) const override;
};
