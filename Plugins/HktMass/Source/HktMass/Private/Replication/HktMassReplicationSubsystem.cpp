// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassReplicationSubsystem.h"
#include "HktMassClientBubbleInfo.h"
#include "MassReplicationSubsystem.h"

void UHktMassReplicationSubsystem::Initialize(FSubsystemCollectionBase & Collection)
{
	Super::Initialize(Collection);
	
	UMassReplicationSubsystem* ReplicationSubsystem = Collection.InitializeDependency<UMassReplicationSubsystem>();
	check(ReplicationSubsystem);
	ReplicationSubsystem->RegisterBubbleInfoClass(AHktMassClientBubbleInfo::StaticClass());
}

void UHktMassReplicationSubsystem::Deinitialize()
{
	Super::Deinitialize();
}
