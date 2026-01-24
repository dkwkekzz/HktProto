// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "FlowDefinitionRegistry.h"

DEFINE_LOG_CATEGORY_STATIC(LogFlowRegistry, Log, All);

TMap<FGameplayTag, TSharedPtr<IFlowDefinition>>& FFlowDefinitionRegistry::GetRegistry()
{
	static TMap<FGameplayTag, TSharedPtr<IFlowDefinition>> Registry;
	return Registry;
}

TMap<FGameplayTag, IFlowDefinition*>& FFlowDefinitionRegistry::GetLookupCache()
{
	static TMap<FGameplayTag, IFlowDefinition*> Cache;
	return Cache;
}

bool FFlowDefinitionRegistry::Register(TSharedPtr<IFlowDefinition> Definition)
{
	if (!Definition.IsValid())
	{
		UE_LOG(LogFlowRegistry, Error, TEXT("Attempted to register null flow definition"));
		return false;
	}

	FGameplayTag Tag = Definition->GetEventTag();
	if (!Tag.IsValid())
	{
		UE_LOG(LogFlowRegistry, Error, TEXT("Flow definition has invalid tag"));
		return false;
	}

	TMap<FGameplayTag, TSharedPtr<IFlowDefinition>>& Registry = GetRegistry();
	
	if (Registry.Contains(Tag))
	{
		UE_LOG(LogFlowRegistry, Warning, TEXT("Flow definition for tag '%s' already registered, replacing"), *Tag.ToString());
	}

	Registry.Add(Tag, Definition);
	
	// Clear cache since we added a new definition
	GetLookupCache().Empty();

	UE_LOG(LogFlowRegistry, Log, TEXT("Registered flow definition for tag: %s"), *Tag.ToString());
	return true;
}

void FFlowDefinitionRegistry::Unregister(FGameplayTag Tag)
{
	TMap<FGameplayTag, TSharedPtr<IFlowDefinition>>& Registry = GetRegistry();
	
	if (Registry.Remove(Tag) > 0)
	{
		// Clear cache since we removed a definition
		GetLookupCache().Empty();
		UE_LOG(LogFlowRegistry, Log, TEXT("Unregistered flow definition for tag: %s"), *Tag.ToString());
	}
}

IFlowDefinition* FFlowDefinitionRegistry::Find(FGameplayTag Tag)
{
	if (!Tag.IsValid())
	{
		return nullptr;
	}

	// Check cache first
	TMap<FGameplayTag, IFlowDefinition*>& Cache = GetLookupCache();
	if (IFlowDefinition** Cached = Cache.Find(Tag))
	{
		return *Cached;
	}

	TMap<FGameplayTag, TSharedPtr<IFlowDefinition>>& Registry = GetRegistry();

	// Try exact match first
	if (TSharedPtr<IFlowDefinition>* Found = Registry.Find(Tag))
	{
		IFlowDefinition* Result = Found->Get();
		Cache.Add(Tag, Result);
		return Result;
	}

	// Try parent tags (from most specific to least specific)
	FGameplayTagContainer ParentTags;
	Tag.GetGameplayTagParents(ParentTags);
	
	// Sort by depth (most specific first)
	TArray<FGameplayTag> SortedParents;
	for (const FGameplayTag& ParentTag : ParentTags)
	{
		SortedParents.Add(ParentTag);
	}
	
	// Sort by tag depth (more dots = more specific)
	SortedParents.Sort([](const FGameplayTag& A, const FGameplayTag& B) {
		FString AStr = A.ToString();
		FString BStr = B.ToString();
		int32 ADepth = 0, BDepth = 0;
		for (TCHAR C : AStr) if (C == '.') ADepth++;
		for (TCHAR C : BStr) if (C == '.') BDepth++;
		return ADepth > BDepth; // Higher depth first
	});

	for (const FGameplayTag& ParentTag : SortedParents)
	{
		if (TSharedPtr<IFlowDefinition>* Found = Registry.Find(ParentTag))
		{
			IFlowDefinition* Result = Found->Get();
			Cache.Add(Tag, Result); // Cache the lookup
			return Result;
		}
	}

	// Not found - cache nullptr to avoid repeated lookups
	Cache.Add(Tag, nullptr);
	return nullptr;
}

bool FFlowDefinitionRegistry::IsRegistered(FGameplayTag Tag)
{
	return Find(Tag) != nullptr;
}

TArray<FGameplayTag> FFlowDefinitionRegistry::GetRegisteredTags()
{
	TArray<FGameplayTag> Tags;
	GetRegistry().GetKeys(Tags);
	return Tags;
}

void FFlowDefinitionRegistry::Clear()
{
	GetRegistry().Empty();
	GetLookupCache().Empty();
	UE_LOG(LogFlowRegistry, Log, TEXT("Cleared all flow definitions"));
}

int32 FFlowDefinitionRegistry::GetRegisteredCount()
{
	return GetRegistry().Num();
}
