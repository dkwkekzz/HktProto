// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassSquadLeader.h"
#include "HktMassSquadSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

AHktMassSquadLeader::AHktMassSquadLeader()
{
	bReplicates = true;
	// 움직임도 복제되어야 함
	SetReplicateMovement(true); 
}

void AHktMassSquadLeader::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHktMassSquadLeader, SquadID);
}

void AHktMassSquadLeader::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UHktMassSquadSubsystem* Subsystem = World->GetSubsystem<UHktMassSquadSubsystem>())
		{
			Subsystem->RegisterSquadLeader(SquadID, this);
		}
	}
}

void AHktMassSquadLeader::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UHktMassSquadSubsystem* Subsystem = World->GetSubsystem<UHktMassSquadSubsystem>())
		{
			Subsystem->UnregisterSquadLeader(SquadID);
		}
	}

	Super::EndPlay(EndPlayReason);
}
