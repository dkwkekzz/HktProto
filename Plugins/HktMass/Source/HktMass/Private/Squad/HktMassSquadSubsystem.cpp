// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassSquadSubsystem.h"

void UHktMassSquadSubsystem::Initialize(FSubsystemCollectionBase & Collection)
{
	Super::Initialize(Collection);
}

void UHktMassSquadSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UHktMassSquadSubsystem::RegisterSquadLeader(int32 SquadID, AHktMassSquadLeader* Leader)
{
	if (Leader)
	{
		SquadLeaderMap.Add(SquadID, Leader);
	}
}

void UHktMassSquadSubsystem::UnregisterSquadLeader(int32 SquadID)
{
	SquadLeaderMap.Remove(SquadID);
}

AHktMassSquadLeader* UHktMassSquadSubsystem::GetSquadLeader(int32 SquadID) const
{
	AHktMassSquadLeader* const* FoundLeader = SquadLeaderMap.Find(SquadID);
	return FoundLeader ? *FoundLeader : nullptr;
}