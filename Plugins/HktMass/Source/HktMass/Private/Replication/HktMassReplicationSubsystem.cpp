// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassReplicationSubsystem.h"
#include "HktMassClientBubbleInfo.h"
#include "HktMassSquadClientBubbleInfo.h"
#include "MassReplicationSubsystem.h"

void UHktMassReplicationSubsystem::Initialize(FSubsystemCollectionBase & Collection)
{
	Super::Initialize(Collection);
	
	UMassReplicationSubsystem* ReplicationSubsystem = Collection.InitializeDependency<UMassReplicationSubsystem>();
	check(ReplicationSubsystem);
	
	// 기본 Agent BubbleInfo 등록
	ReplicationSubsystem->RegisterBubbleInfoClass(AHktMassClientBubbleInfo::StaticClass());
	
	// Squad BubbleInfo 등록
	ReplicationSubsystem->RegisterBubbleInfoClass(AHktMassSquadClientBubbleInfo::StaticClass());
}

void UHktMassReplicationSubsystem::Deinitialize()
{
	Super::Deinitialize();
}
