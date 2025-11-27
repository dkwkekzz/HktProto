// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassSquadClientBubbleInfo.h"
#include "HktMassSquadFragments.h"
#include "MassCommonFragments.h"
#include "MassEntityView.h"
#include "Net/UnrealNetwork.h"
#include "MassExecutionContext.h"

//----------------------------------------------------------------------//
// FHktMassSquadClientBubbleHandler
//----------------------------------------------------------------------//

#if UE_REPLICATION_COMPILE_CLIENT_CODE
void FHktMassSquadClientBubbleHandler::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	auto AddRequirementsForSpawnQuery = [](FMassEntityQuery& InQuery)
	{
		InQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
		InQuery.AddRequirement<FHktMassSquadFragment>(EMassFragmentAccess::ReadWrite);
	};

	auto CacheFragmentViewsForSpawnQuery = [](FMassExecutionContext& InExecContext) 
	{
	};

	auto SetSpawnedEntityData = [this](const FMassEntityView& EntityView, const FHktReplicatedSquadAgent& Agent, const int32 EntityIdx)
	{
		UpdateEntityFragments(EntityView, Agent);
	};

	auto SetModifiedEntityData = [this](const FMassEntityView& EntityView, const FHktReplicatedSquadAgent& Agent)
	{
		UpdateEntityFragments(EntityView, Agent);
	};

	PostReplicatedAddHelper(AddedIndices, AddRequirementsForSpawnQuery, CacheFragmentViewsForSpawnQuery, SetSpawnedEntityData, SetModifiedEntityData);
}

void FHktMassSquadClientBubbleHandler::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	auto SetModifiedEntityData = [this](const FMassEntityView& EntityView, const FHktReplicatedSquadAgent& Agent)
	{
		UpdateEntityFragments(EntityView, Agent);
	};

	PostReplicatedChangeHelper(ChangedIndices, SetModifiedEntityData);
}

void FHktMassSquadClientBubbleHandler::UpdateEntityFragments(const FMassEntityView& EntityView, const FHktReplicatedSquadAgent& Agent)
{
	// 1. Transform 업데이트
	if (FTransformFragment* TransformFrag = EntityView.GetFragmentDataPtr<FTransformFragment>())
	{
		TransformFrag->GetMutableTransform().SetLocation(Agent.Position);
	}

	// 2. Squad 정보 업데이트
	if (FHktMassSquadFragment* SquadFrag = EntityView.GetFragmentDataPtr<FHktMassSquadFragment>())
	{
		SquadFrag->SquadState = Agent.SquadState;
		SquadFrag->MemberCount = Agent.MemberCount;
		SquadFrag->MemberConfig = Agent.MemberConfig;
	}
}
#endif // UE_REPLICATION_COMPILE_CLIENT_CODE

#if UE_REPLICATION_COMPILE_SERVER_CODE
FMassReplicatedAgentHandle FHktMassSquadClientBubbleHandler::AddAgent(FMassEntityHandle Entity, FHktReplicatedSquadAgent& Agent)
{
	return Super::AddAgent(Entity, Agent);
}

bool FHktMassSquadClientBubbleHandler::ModifyAgent(FMassReplicatedAgentHandle Handle, const FHktReplicatedSquadAgent& Agent, 
	EMassLOD::Type LOD, double Time, double LastUpdateTime, const float* UpdateIntervals)
{
	// LOD에 따른 업데이트 빈도 조절
	const float UpdateInterval = UpdateIntervals[LOD];
	const double TimeSinceLastUpdate = Time - LastUpdateTime;
	
	if (TimeSinceLastUpdate < UpdateInterval)
	{
		return false; 
	}

	if (AgentHandleManager.IsValidHandle(Handle))
	{
		const FMassAgentLookupData& LookUpData = AgentLookupArray[Handle.GetIndex()];
		FHktReplicatedSquadAgentArrayItem& Item = (*Agents)[LookUpData.AgentsIdx];

		// Agent 데이터 업데이트
		Item.Agent = Agent;

		// Serializer에 변경사항 알림
		Serializer->MarkItemDirty(Item);

		return true;
	}
	
	return false;
}

void FHktMassSquadClientBubbleHandler::RemoveAgent(FMassReplicatedAgentHandle Handle)
{
	Super::RemoveAgentChecked(Handle);
}
#endif // UE_REPLICATION_COMPILE_SERVER_CODE

//----------------------------------------------------------------------//
// FHktMassSquadClientBubbleSerializer
//----------------------------------------------------------------------//

FHktMassSquadClientBubbleSerializer::FHktMassSquadClientBubbleSerializer()
{
	Bubble.Initialize(Agents, *this);
}

bool FHktMassSquadClientBubbleSerializer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FHktReplicatedSquadAgentArrayItem, FHktMassSquadClientBubbleSerializer>(Agents, DeltaParams, *this);
}

//----------------------------------------------------------------------//
// AHktMassSquadClientBubbleInfo
//----------------------------------------------------------------------//

AHktMassSquadClientBubbleInfo::AHktMassSquadClientBubbleInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Serializer 등록 (기본 생성자에서 처리됨)
	Serializers.Add(&SquadSerializer);
}

void AHktMassSquadClientBubbleInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHktMassSquadClientBubbleInfo, SquadSerializer);
}
