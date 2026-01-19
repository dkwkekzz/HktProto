// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Core/HktSimulationResult.h"

class FHktFlowBuilder;
struct FHktIntentEvent;
class FHktEntityManager;

/**
 * Interface for defining how IntentEvents are converted to bytecode
 * Each flow type implements this interface to specify its behavior
 */
class HKTSIMULATION_API IFlowDefinition
{
public:
	virtual ~IFlowDefinition() = default;

	/**
	 * Builds bytecode for the given event (NEW Result-based API)
	 * @param Builder - The bytecode builder to use
	 * @param Event - The intent event to process
	 * @param EntityManager - Access to entity system for handle resolution
	 * @return Result indicating success or failure with error details
	 */
	virtual FSimulationResult BuildBytecodeWithResult(FHktFlowBuilder& Builder, const FHktIntentEvent& Event, FHktEntityManager* EntityManager)
	{
		// Default implementation delegates to legacy API
		bool bSuccess = BuildBytecode(Builder, Event, EntityManager);
		return bSuccess ? FSimulationResult::Success() : 
			FSimulationResult::Failure(ESimulationError::BuildFailed, TEXT("BuildBytecode returned false"));
	}

	/**
	 * Builds bytecode for the given event (LEGACY API - for backward compatibility)
	 * @param Builder - The bytecode builder to use
	 * @param Event - The intent event to process
	 * @param EntityManager - Access to entity system for handle resolution
	 * @return true if bytecode was successfully built
	 */
	virtual bool BuildBytecode(FHktFlowBuilder& Builder, const FHktIntentEvent& Event, FHktEntityManager* EntityManager) = 0;

	/**
	 * Returns the GameplayTag this flow handles
	 */
	virtual FGameplayTag GetEventTag() const = 0;

	/**
	 * Optional: Returns the priority for this flow definition
	 * Higher priority flows are checked first (default: 100)
	 */
	virtual int32 GetPriority() const { return 100; }

	/**
	 * Optional: Validation before building bytecode
	 * @return true if the event is valid for this flow
	 */
	virtual bool ValidateEvent(const FHktIntentEvent& Event) const { return true; }
};
