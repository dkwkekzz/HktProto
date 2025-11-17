// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassNpcClientBubbleInfo.h"
#include "HktMassNpcReplicationHelpers.h"
#include "Net/UnrealNetwork.h"

//----------------------------------------------------------------------//
// FHktMassNpcClientBubbleHandler
//----------------------------------------------------------------------//

#if UE_REPLICATION_COMPILE_CLIENT_CODE
void FHktMassNpcClientBubbleHandler::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	// Helper???ÑÏûÑ
	FHktMassNpcClientReplicationHelper::HandleItemRemoved(RemovedIndices, *Agents, *Serializer);
}

void FHktMassNpcClientBubbleHandler::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	// Helper???ÑÏûÑ
	FHktMassNpcClientReplicationHelper::HandleItemAdded(AddedIndices, *Agents, *Serializer);
}

void FHktMassNpcClientBubbleHandler::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	// Helper???ÑÏûÑ
	FHktMassNpcClientReplicationHelper::HandleItemChanged(ChangedIndices, *Agents, *Serializer);
}
#endif // UE_REPLICATION_COMPILE_CLIENT_CODE

#if UE_REPLICATION_COMPILE_SERVER_CODE
FMassReplicatedAgentHandle FHktMassNpcClientBubbleHandler::AddAgent(
	FMassEntityHandle Entity, 
	FHktReplicatedNpcAgent& Agent)
{
	// Serializer??Agents Î∞∞Ïó¥??Í∞Ä?∏Ïò¥
	check(Agents);
	check(Serializer);
	
	// Helper???ÑÏûÑ
	return FHktMassNpcServerReplicationHelper::AddAgent(*Agents, *Serializer, Entity, Agent);
}

bool FHktMassNpcClientBubbleHandler::ModifyAgent(
	FMassReplicatedAgentHandle Handle, 
	const FHktReplicatedNpcAgent& Agent,
	EMassLOD::Type LOD, 
	double Time, 
	double LastUpdateTime, 
	const float* UpdateIntervals)
{
	check(Agents);
	check(Serializer);
	
	// Helper???ÑÏûÑ
	return FHktMassNpcServerReplicationHelper::ModifyAgent(*Agents, *Serializer, Handle, Agent, 
		LOD, Time, LastUpdateTime, UpdateIntervals);
}

void FHktMassNpcClientBubbleHandler::RemoveAgent(FMassReplicatedAgentHandle Handle)
{
	check(Agents);
	check(Serializer);
	
	// Helper???ÑÏûÑ
	FHktMassNpcServerReplicationHelper::RemoveAgent(*Agents, *Serializer, Handle);
}
#endif // UE_REPLICATION_COMPILE_SERVER_CODE

//----------------------------------------------------------------------//
// FHktMassNpcClientBubbleSerializer
//----------------------------------------------------------------------//

FHktMassNpcClientBubbleSerializer::FHktMassNpcClientBubbleSerializer()
{
	// Handler?Ä FastArray ?∞Í≤∞
	Bubble.Initialize(Agents, *this);
}

bool FHktMassNpcClientBubbleSerializer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	// FastArray ?∏Ì? ÏßÅÎ†¨???òÌñâ
	return FFastArraySerializer::FastArrayDeltaSerialize<FHktReplicatedNpcAgentArrayItem, FHktMassNpcClientBubbleSerializer>(
		Agents, DeltaParams, *this);
}

//----------------------------------------------------------------------//
// AHktMassNpcClientBubbleInfo
//----------------------------------------------------------------------//

AHktMassNpcClientBubbleInfo::AHktMassNpcClientBubbleInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Serializer Î∞∞Ïó¥???±Î°ù
	Serializers.Add(&NpcSerializer);
}

void AHktMassNpcClientBubbleInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// NpcSerializerÎ•?Î≥µÏ†ú ?Ä?ÅÏúºÎ°??±Î°ù
	DOREPLIFETIME(AHktMassNpcClientBubbleInfo, NpcSerializer);
}

