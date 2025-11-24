// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassSquadSubsystem.h"
#include "HktMassSquadCommandComponent.h"

void UHktMassSquadSubsystem::Initialize(FSubsystemCollectionBase & Collection)
{
	Super::Initialize(Collection);
}

void UHktMassSquadSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UHktMassSquadSubsystem::RegisterSquadCommandComponent(int32 SquadID, UHktMassSquadCommandComponent* SquadComponent)
{
	if (SquadComponent)
	{
		SquadComponentMap.Add(SquadID, SquadComponent);
	}
}

void UHktMassSquadSubsystem::UnregisterSquadCommandComponent(int32 SquadID)
{
	SquadComponentMap.Remove(SquadID);
}

UHktMassSquadCommandComponent* UHktMassSquadSubsystem::GetSquadCommandComponent(int32 SquadID) const
{
	UHktMassSquadCommandComponent* const* FoundComponent = SquadComponentMap.Find(SquadID);
	return FoundComponent ? *FoundComponent : nullptr;
}