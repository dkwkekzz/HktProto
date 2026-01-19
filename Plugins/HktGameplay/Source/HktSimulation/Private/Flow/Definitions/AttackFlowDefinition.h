// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Flow/IFlowDefinition.h"

/**
 * Flow definition for basic attack events
 * Handles Hkt.Event.Action.Attack and child tags
 */
class FAttackFlowDefinition : public IFlowDefinition
{
public:
	virtual bool BuildBytecode(FHktFlowBuilder& Builder, const FHktIntentEvent& Event, FHktEntityManager* EntityManager) override;
	virtual FGameplayTag GetEventTag() const override;
	virtual bool ValidateEvent(const FHktIntentEvent& Event) const override;
};
