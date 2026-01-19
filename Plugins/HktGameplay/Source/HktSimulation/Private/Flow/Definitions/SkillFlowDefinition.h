// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Flow/IFlowDefinition.h"

/**
 * Flow definition for skill/ability events
 * Handles Hkt.Event.Ability and child tags
 */
class FSkillFlowDefinition : public IFlowDefinition
{
public:
	virtual bool BuildBytecode(FHktFlowBuilder& Builder, const FHktIntentEvent& Event, FHktEntityManager* EntityManager) override;
	virtual FGameplayTag GetEventTag() const override;

private:
	bool BuildFireballSkill(FHktFlowBuilder& Builder, const FHktIntentEvent& Event);
};
