// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "IFlowDefinition.h"

/**
 * Central registry for Flow Definitions
 * Manages the mapping between GameplayTags and their flow implementations
 * Uses Meyers Singleton pattern to avoid static initialization order issues
 */
class HKTSIMULATION_API FFlowDefinitionRegistry
{
public:
	/**
	 * Register a flow definition
	 * @param Definition - The flow definition to register
	 * @return true if registration was successful
	 */
	static bool Register(TSharedPtr<IFlowDefinition> Definition);

	/**
	 * Unregister a flow definition
	 * @param Tag - The tag of the flow to unregister
	 */
	static void Unregister(FGameplayTag Tag);

	/**
	 * Find a flow definition by tag
	 * Searches for exact match first, then checks parent tags
	 * @param Tag - The event tag to find a flow for
	 * @return The flow definition, or nullptr if not found
	 */
	static IFlowDefinition* Find(FGameplayTag Tag);

	/**
	 * Check if a flow is registered for a tag
	 */
	static bool IsRegistered(FGameplayTag Tag);

	/**
	 * Get all registered tags
	 */
	static TArray<FGameplayTag> GetRegisteredTags();

	/**
	 * Clear all registered flows (useful for testing)
	 */
	static void Clear();

	/**
	 * Get statistics about registered flows
	 */
	static int32 GetRegisteredCount();

private:
	/**
	 * Internal registry storage (Meyers Singleton)
	 * Prevents static initialization order fiasco
	 */
	static TMap<FGameplayTag, TSharedPtr<IFlowDefinition>>& GetRegistry();

	/**
	 * Cache for parent tag lookups
	 */
	static TMap<FGameplayTag, IFlowDefinition*>& GetLookupCache();
};

/**
 * Helper macro for auto-registering flow definitions
 * Usage: REGISTER_FLOW_DEFINITION(FAttackFlowDefinition)
 */
#define REGISTER_FLOW_DEFINITION(FlowClass) \
	namespace { \
		struct F##FlowClass##AutoRegister { \
			F##FlowClass##AutoRegister() { \
				FFlowDefinitionRegistry::Register(MakeShared<FlowClass>()); \
			} \
		}; \
		static F##FlowClass##AutoRegister G##FlowClass##AutoRegister; \
	}
