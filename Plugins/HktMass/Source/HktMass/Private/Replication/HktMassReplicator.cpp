// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassReplicator.h"
#include "HktMassClientBubbleInfo.h"
#include "HktMassCommonFragments.h"
#include "Movement/HktMassMoveToTargetTrait.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassExecutionContext.h"
#include "MassReplicationSubsystem.h"

UHktMassReplicator::UHktMassReplicator()
{
}

void UHktMassReplicator::AddRequirements(FMassEntityQuery& EntityQuery)
{
	// 복제???�요??Fragment ?�구?�항 추�?
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHktMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHktNpcStateFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHktNpcCombatFragment>(EMassFragmentAccess::ReadOnly);
	// ?�고: Animation Fragment??복제?��? ?�음 (클라?�언???�체 계산)
}

void UHktMassReplicator::ProcessClientReplication(
	FMassExecutionContext& Context, 
	FMassReplicationContext& ReplicationContext)
{
	// CalculateClientReplication 
	// ???�수??MassReplicationProcessor.h???�의?�어 ?�으�?
	// AddEntity, ModifyEntity, RemoveEntity 콜백??받아 �??�티?��? 처리??
	
	UMassReplicatorBase::CalculateClientReplication<FHktReplicatedNpcAgentArrayItem>(
		Context, 
		ReplicationContext,
		
		// CacheViews: Fragment View�?캐시 (?�재???�용?��? ?�음)
		[](FMassExecutionContext& Context)
		{
			// ?�요??Fragment View�?미리 캐시?????�음
		},
		
		// AddEntity: ???�티??추�?
		[](FMassExecutionContext& Context, int32 EntityIdx, FHktReplicatedNpcAgent& Agent, FMassClientHandle ClientHandle) -> FMassReplicatedAgentHandle
		{
			// Fragment ?�이???�집
			const TConstArrayView<FReplicationTemplateIDFragment> TemplateIDList = Context.GetFragmentView<FReplicationTemplateIDFragment>();
			const TConstArrayView<FMassNetworkIDFragment> NetworkIDList = Context.GetFragmentView<FMassNetworkIDFragment>();
			const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
			const TConstArrayView<FHktMassVelocityFragment> VelocityList = Context.GetFragmentView<FHktMassVelocityFragment>();
			const TConstArrayView<FHktNpcStateFragment> StateList = Context.GetFragmentView<FHktNpcStateFragment>();
			const TConstArrayView<FHktNpcCombatFragment> CombatList = Context.GetFragmentView<FHktNpcCombatFragment>();
			
			// Agent ?�이???�정
			Agent.SetTemplateID(TemplateIDList[EntityIdx].ID);
			Agent.SetNetID(NetworkIDList[EntityIdx].NetID);
			const FTransform& Transform = TransformList[EntityIdx].GetTransform();
			Agent.Position = Transform.GetLocation();
			Agent.Rotation = Transform.GetRotation().Rotator();
			Agent.Velocity = VelocityList[EntityIdx].Value;
			Agent.CurrentState = StateList[EntityIdx].CurrentState;
			Agent.CurrentHealth = CombatList[EntityIdx].CurrentHealth;
			// ?�고: Animation??복제?��? ?�음 - ?�라?�언?�에??Velocity + State�??�탕?�로 계산
			
			// BubbleInfo 가?�오�?
			FMassReplicationSharedFragment& RepSharedFragment = Context.GetMutableSharedFragment<FMassReplicationSharedFragment>();
			AHktMassClientBubbleInfo& BubbleInfo = RepSharedFragment.GetTypedClientBubbleInfoChecked<AHktMassClientBubbleInfo>(ClientHandle);
			
			// Handler�??�해 Agent 추�?
			FMassEntityHandle Entity = Context.GetEntity(EntityIdx);
			return BubbleInfo.GetNpcSerializer().GetBubbleHandler().AddAgent(Entity, Agent);
		},
		
		// ModifyEntity: 기존 ?�티???�정
		[](FMassExecutionContext& Context, int32 EntityIdx, EMassLOD::Type LOD, double Time, FMassReplicatedAgentHandle Handle, FMassClientHandle ClientHandle)
		{
			// Fragment ?�이???�집
			const TConstArrayView<FReplicationTemplateIDFragment> TemplateIDList = Context.GetFragmentView<FReplicationTemplateIDFragment>();
			const TConstArrayView<FMassNetworkIDFragment> NetworkIDList = Context.GetFragmentView<FMassNetworkIDFragment>();
			const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
			const TConstArrayView<FHktMassVelocityFragment> VelocityList = Context.GetFragmentView<FHktMassVelocityFragment>();
			const TConstArrayView<FHktNpcStateFragment> StateList = Context.GetFragmentView<FHktNpcStateFragment>();
			const TConstArrayView<FHktNpcCombatFragment> CombatList = Context.GetFragmentView<FHktNpcCombatFragment>();
			const TArrayView<FMassReplicatedAgentFragment> ReplicatedAgentList = Context.GetMutableFragmentView<FMassReplicatedAgentFragment>();
			
			// Agent ?�이???�정
			FHktReplicatedNpcAgent Agent;
			Agent.SetTemplateID(TemplateIDList[EntityIdx].ID);
			Agent.SetNetID(NetworkIDList[EntityIdx].NetID);
			const FTransform& Transform = TransformList[EntityIdx].GetTransform();
			Agent.Position = Transform.GetLocation();
			Agent.Rotation = Transform.GetRotation().Rotator();
			Agent.Velocity = VelocityList[EntityIdx].Value;
			Agent.CurrentState = StateList[EntityIdx].CurrentState;
			Agent.CurrentHealth = CombatList[EntityIdx].CurrentHealth;
			// ?�고: Animation??복제?��? ?�음 - ?�라?�언?�에??Velocity + State�??�탕?�로 계산
			
			// BubbleInfo 가?�오�?
			FMassReplicationSharedFragment& RepSharedFragment = Context.GetMutableSharedFragment<FMassReplicationSharedFragment>();
			AHktMassClientBubbleInfo& BubbleInfo = RepSharedFragment.GetTypedClientBubbleInfoChecked<AHktMassClientBubbleInfo>(ClientHandle);
			
			// LOD�??�데?�트 간격
			const FMassReplicationParameters& RepParams = Context.GetConstSharedFragment<FMassReplicationParameters>();
			const FMassReplicatedAgentData& AgentData = ReplicatedAgentList[EntityIdx].AgentData;
			
			// Handler�??�해 Agent ?�정
			BubbleInfo.GetNpcSerializer().GetBubbleHandler().ModifyAgent(
				Handle, Agent, LOD, Time, AgentData.LastUpdateTime, RepParams.UpdateInterval);
		},
		
		// RemoveEntity: ?�티???�거
		[](FMassExecutionContext& Context, FMassReplicatedAgentHandle Handle, FMassClientHandle ClientHandle)
		{
			// BubbleInfo 가?�오�?
			FMassReplicationSharedFragment& RepSharedFragment = Context.GetMutableSharedFragment<FMassReplicationSharedFragment>();
			AHktMassClientBubbleInfo& BubbleInfo = RepSharedFragment.GetTypedClientBubbleInfoChecked<AHktMassClientBubbleInfo>(ClientHandle);
			
			// Handler�??�해 Agent ?�거
			BubbleInfo.GetNpcSerializer().GetBubbleHandler().RemoveAgent(Handle);
		}
	);
}

