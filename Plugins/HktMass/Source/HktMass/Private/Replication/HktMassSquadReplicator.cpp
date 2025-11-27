// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassSquadReplicator.h"
#include "HktMassSquadClientBubbleInfo.h"
#include "HktMassSquadFragments.h"
#include "MassCommonFragments.h"
#include "MassReplicationSubsystem.h"
#include "MassExecutionContext.h"

UHktMassSquadReplicator::UHktMassSquadReplicator()
{
}

void UHktMassSquadReplicator::AddRequirements(FMassEntityQuery& EntityQuery)
{
	// 복제에 필요한 Fragment 요구사항 추가
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHktMassSquadFragment>(EMassFragmentAccess::ReadOnly);
}

void UHktMassSquadReplicator::ProcessClientReplication(FMassExecutionContext& Context, FMassReplicationContext& ReplicationContext)
{
	UMassReplicatorBase::CalculateClientReplication<FHktReplicatedSquadAgentArrayItem>(
		Context, 
		ReplicationContext,
		[](FMassExecutionContext& Context)
		{
			// 필요한 경우 Fragment View 캐싱
		},
		[](FMassExecutionContext& Context, int32 EntityIdx, FHktReplicatedSquadAgent& Agent, FMassClientHandle ClientHandle) -> FMassReplicatedAgentHandle
		{
			// Fragment 데이터 수집
			const TConstArrayView<FReplicationTemplateIDFragment> TemplateIDList = Context.GetFragmentView<FReplicationTemplateIDFragment>();
			const TConstArrayView<FMassNetworkIDFragment> NetworkIDList = Context.GetFragmentView<FMassNetworkIDFragment>();
			const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
			const TConstArrayView<FHktMassSquadFragment> SquadFragments = Context.GetFragmentView<FHktMassSquadFragment>();

			// Agent 데이터 설정
			Agent.SetTemplateID(TemplateIDList[EntityIdx].ID);
			Agent.SetNetID(NetworkIDList[EntityIdx].NetID);
			
			const FTransform& Transform = TransformList[EntityIdx].GetTransform();
			Agent.Position = Transform.GetLocation();

			const FHktMassSquadFragment& SquadFrag = SquadFragments[EntityIdx];
			Agent.SquadState = SquadFrag.SquadState;
			Agent.MemberCount = SquadFrag.MemberCount;
			Agent.MemberConfig = SquadFrag.MemberConfig;

			// BubbleInfo 가져오기
			FMassReplicationSharedFragment& RepSharedFragment = Context.GetMutableSharedFragment<FMassReplicationSharedFragment>();
			AHktMassSquadClientBubbleInfo& BubbleInfo = RepSharedFragment.GetTypedClientBubbleInfoChecked<AHktMassSquadClientBubbleInfo>(ClientHandle);

			// Handler를 통해 Agent 추가
			FMassEntityHandle Entity = Context.GetEntity(EntityIdx);
			return BubbleInfo.GetSquadSerializer().GetBubbleHandler().AddAgent(Entity, Agent);
		},
		[](FMassExecutionContext& Context, int32 EntityIdx, EMassLOD::Type LOD, double Time, FMassReplicatedAgentHandle Handle, FMassClientHandle ClientHandle)
		{
			// Fragment 데이터 수집
			const TConstArrayView<FReplicationTemplateIDFragment> TemplateIDList = Context.GetFragmentView<FReplicationTemplateIDFragment>();
			const TConstArrayView<FMassNetworkIDFragment> NetworkIDList = Context.GetFragmentView<FMassNetworkIDFragment>();
			const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
			const TConstArrayView<FHktMassSquadFragment> SquadFragments = Context.GetFragmentView<FHktMassSquadFragment>();
			const TArrayView<FMassReplicatedAgentFragment> ReplicatedAgentList = Context.GetMutableFragmentView<FMassReplicatedAgentFragment>();

			// Agent 데이터 설정
			FHktReplicatedSquadAgent Agent;
			Agent.SetTemplateID(TemplateIDList[EntityIdx].ID);
			Agent.SetNetID(NetworkIDList[EntityIdx].NetID);

			const FTransform& Transform = TransformList[EntityIdx].GetTransform();
			Agent.Position = Transform.GetLocation();

			const FHktMassSquadFragment& SquadFrag = SquadFragments[EntityIdx];
			Agent.SquadState = SquadFrag.SquadState;
			Agent.MemberCount = SquadFrag.MemberCount;
			Agent.MemberConfig = SquadFrag.MemberConfig;

			// BubbleInfo 가져오기
			FMassReplicationSharedFragment& RepSharedFragment = Context.GetMutableSharedFragment<FMassReplicationSharedFragment>();
			AHktMassSquadClientBubbleInfo& BubbleInfo = RepSharedFragment.GetTypedClientBubbleInfoChecked<AHktMassSquadClientBubbleInfo>(ClientHandle);

			// LOD 및 업데이트 간격
			const FMassReplicationParameters& RepParams = Context.GetConstSharedFragment<FMassReplicationParameters>();
			const FMassReplicatedAgentData& AgentData = ReplicatedAgentList[EntityIdx].AgentData;

			// Handler를 통해 Agent 수정
			BubbleInfo.GetSquadSerializer().GetBubbleHandler().ModifyAgent(
				Handle, Agent, LOD, Time, AgentData.LastUpdateTime, RepParams.UpdateInterval);
		},
		[](FMassExecutionContext& Context, FMassReplicatedAgentHandle Handle, FMassClientHandle ClientHandle)
		{
			// BubbleInfo 가져오기
			FMassReplicationSharedFragment& RepSharedFragment = Context.GetMutableSharedFragment<FMassReplicationSharedFragment>();
			AHktMassSquadClientBubbleInfo& BubbleInfo = RepSharedFragment.GetTypedClientBubbleInfoChecked<AHktMassSquadClientBubbleInfo>(ClientHandle);

			// Handler를 통해 Agent 제거
			BubbleInfo.GetSquadSerializer().GetBubbleHandler().RemoveAgent(Handle);
		}
	);
}
