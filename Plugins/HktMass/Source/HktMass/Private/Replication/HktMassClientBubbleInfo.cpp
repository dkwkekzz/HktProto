// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassClientBubbleInfo.h"
#include "HktMassMovementFragments.h"
#include "HktMassPhysicsFragments.h"
#include "Net/UnrealNetwork.h"
#include "MassEntityView.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "HktMassCommonFragments.h"
#include "MassExecutionContext.h"

//----------------------------------------------------------------------//
// FHktMassClientBubbleHandler
//----------------------------------------------------------------------//

#if UE_REPLICATION_COMPILE_CLIENT_CODE
void FHktMassClientBubbleHandler::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	auto AddRequirementsForSpawnQuery = [](FMassEntityQuery& InQuery)
	{
		InQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
		InQuery.AddRequirement<FHktMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
		InQuery.AddRequirement<FHktNpcStateFragment>(EMassFragmentAccess::ReadWrite);
		InQuery.AddRequirement<FHktNpcCombatFragment>(EMassFragmentAccess::ReadWrite);
	};

	auto CacheFragmentViewsForSpawnQuery = [](FMassExecutionContext& InExecContext) 
	{
		// 캐싱이 필요한 경우 여기에 구현 (현재는 불필요)
	};

	auto SetSpawnedEntityData = [this](const FMassEntityView& EntityView, const FHktReplicatedNpcAgent& Agent, const int32 EntityIdx)
	{
		UpdateEntityFragments(EntityView, Agent);
	};

	auto SetModifiedEntityData = [this](const FMassEntityView& EntityView, const FHktReplicatedNpcAgent& Agent)
	{
		UpdateEntityFragments(EntityView, Agent);
	};

	PostReplicatedAddHelper(AddedIndices, AddRequirementsForSpawnQuery, CacheFragmentViewsForSpawnQuery, SetSpawnedEntityData, SetModifiedEntityData);
}

void FHktMassClientBubbleHandler::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	auto SetModifiedEntityData = [this](const FMassEntityView& EntityView, const FHktReplicatedNpcAgent& Agent)
	{
		UpdateEntityFragments(EntityView, Agent);
	};

	PostReplicatedChangeHelper(ChangedIndices, SetModifiedEntityData);
}

void FHktMassClientBubbleHandler::UpdateEntityFragments(const FMassEntityView& EntityView, const FHktReplicatedNpcAgent& Agent)
{
	// Transform 업데이트
	if (FTransformFragment* TransformFragment = EntityView.GetFragmentDataPtr<FTransformFragment>())
	{
		TransformFragment->GetMutableTransform().SetLocation(Agent.Position);
		TransformFragment->GetMutableTransform().SetRotation(Agent.Rotation.Quaternion());
	}

	// Velocity 업데이트 (보간을 위한 중요)
	if (FHktMassVelocityFragment* VelocityFragment = EntityView.GetFragmentDataPtr<FHktMassVelocityFragment>())
	{
		VelocityFragment->Value = Agent.Velocity;
	}

	// State 업데이트
	if (FHktNpcStateFragment* StateFragment = EntityView.GetFragmentDataPtr<FHktNpcStateFragment>())
	{
		StateFragment->CurrentState = Agent.CurrentState;
	}

	// Combat (Health) 업데이트
	if (FHktNpcCombatFragment* CombatFragment = EntityView.GetFragmentDataPtr<FHktNpcCombatFragment>())
	{
		CombatFragment->CurrentHealth = Agent.CurrentHealth;
	}

	// 참고: Animation은 복제되지 않음
	// → HktMassAnimationProcessor가 Velocity와 State를 기반으로 클라이언트에서 자체 계산
}
#endif // UE_REPLICATION_COMPILE_CLIENT_CODE

#if UE_REPLICATION_COMPILE_SERVER_CODE
FMassReplicatedAgentHandle FHktMassClientBubbleHandler::AddAgent(
	FMassEntityHandle Entity, 
	FHktReplicatedNpcAgent& Agent)
{
	// 베이스 클래스의 AddAgent 사용 (Handle 관리, NetworkIDToAgentHandleMap, AgentLookupArray 등 모두 처리)
	return Super::AddAgent(Entity, Agent);
}

bool FHktMassClientBubbleHandler::ModifyAgent(
	FMassReplicatedAgentHandle Handle, 
	const FHktReplicatedNpcAgent& Agent,
	EMassLOD::Type LOD, 
	double Time, 
	double LastUpdateTime, 
	const float* UpdateIntervals)
{
	check(Agents);
	check(Serializer);
	
	// LOD에 따른 업데이트 간격 체크
	const float UpdateInterval = UpdateIntervals[LOD];
	const double TimeSinceLastUpdate = Time - LastUpdateTime;
	
	if (TimeSinceLastUpdate < UpdateInterval)
	{
		return false; // 아직 업데이트할 시간이 아님
	}

	// 베이스 클래스의 AgentLookupArray를 통해 Agent 찾기
	if (AgentHandleManager.IsValidHandle(Handle))
	{
		const FMassAgentLookupData& LookUpData = AgentLookupArray[Handle.GetIndex()];
		FHktReplicatedNpcAgentArrayItem& Item = (*Agents)[LookUpData.AgentsIdx];
		
		// Agent 데이터 업데이트
		Item.Agent = Agent;
		
		// Serializer에 변경사항 알림
		Serializer->MarkItemDirty(Item);
		
		return true;
	}
	
	return false;
}

void FHktMassClientBubbleHandler::RemoveAgent(FMassReplicatedAgentHandle Handle)
{
	// 베이스 클래스의 RemoveAgentChecked 사용 (모든 정리 작업 처리)
	Super::RemoveAgentChecked(Handle);
}
#endif // UE_REPLICATION_COMPILE_SERVER_CODE

//----------------------------------------------------------------------//
// FHktMassClientBubbleSerializer
//----------------------------------------------------------------------//

FHktMassClientBubbleSerializer::FHktMassClientBubbleSerializer()
{
	// Handler?� FastArray ?�결
	Bubble.Initialize(Agents, *this);
}

bool FHktMassClientBubbleSerializer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	// FastArray ?��? 직렬???�행
	return FFastArraySerializer::FastArrayDeltaSerialize<FHktReplicatedNpcAgentArrayItem, FHktMassClientBubbleSerializer>(
		Agents, DeltaParams, *this);
}

//----------------------------------------------------------------------//
// AHktMassClientBubbleInfo
//----------------------------------------------------------------------//

AHktMassClientBubbleInfo::AHktMassClientBubbleInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Serializer 배열???�록
	Serializers.Add(&NpcSerializer);
}

void AHktMassClientBubbleInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// NpcSerializer�?복제 ?�?�으�??�록
	DOREPLIFETIME(AHktMassClientBubbleInfo, NpcSerializer);
}

