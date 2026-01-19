// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Flow/IFlowDefinition.h"

/**
 * Flow definition for movement events
 * Handles Hkt.Event.MoveToLocation and related tags
 */
class FMoveFlowDefinition : public IFlowDefinition
{
public:
	virtual bool BuildBytecode(FHktFlowBuilder& Builder, const FHktIntentEvent& Event, FHktEntityManager* EntityManager) override;
	virtual FGameplayTag GetEventTag() const override;
};
