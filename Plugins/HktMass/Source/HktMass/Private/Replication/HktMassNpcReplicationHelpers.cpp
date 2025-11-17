// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassNpcReplicationHelpers.h"
#include "MassClientBubbleSerializerBase.h"
#include "MassEntityView.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassReplicationSubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "HktMassNpcFragments.h"
#include "MassEntityTemplateRegistry.h"

//----------------------------------------------------------------------//
// FHktMassNpcServerReplicationHelper
//----------------------------------------------------------------------//

FMassReplicatedAgentHandle FHktMassNpcServerReplicationHelper::AddAgent(
	TArray<FHktReplicatedNpcAgentArrayItem>& Agents,
	FMassClientBubbleSerializerBase& Serializer,
	FMassEntityHandle Entity,
	const FHktReplicatedNpcAgent& Agent)
{
	// FastArray????Agent 추�?
	FHktReplicatedNpcAgentArrayItem& NewItem = Agents.AddDefaulted_GetRef();
	NewItem.Agent = Agent;
	
	// Serializer??변경사???�림
	Serializer.MarkItemDirty(NewItem);
	
	// Handle?� FastArray ?�스?�이 ?�동?�로 관�?
	return NewItem.GetHandle();
}

bool FHktMassNpcServerReplicationHelper::ModifyAgent(
	TArray<FHktReplicatedNpcAgentArrayItem>& Agents,
	FMassClientBubbleSerializerBase& Serializer,
	FMassReplicatedAgentHandle Handle,
	const FHktReplicatedNpcAgent& Agent,
	EMassLOD::Type LOD,
	double Time,
	double LastUpdateTime,
	const float* UpdateIntervals)
{
	// LOD???�른 ?�데?�트 간격 체크
	const float UpdateInterval = UpdateIntervals[LOD];
	const double TimeSinceLastUpdate = Time - LastUpdateTime;
	
	if (TimeSinceLastUpdate < UpdateInterval)
	{
		return false; // ?�직 ?�데?�트???�간???�님
	}

	// Handle�?Agent 찾기
	for (FHktReplicatedNpcAgentArrayItem& Item : Agents)
	{
		if (Item.GetHandle() == Handle)
		{
			// Agent ?�이???�데?�트
			Item.Agent = Agent;
			
			// Serializer??변경사???�림
			Serializer.MarkItemDirty(Item);
			
			return true;
		}
	}
	
	return false;
}

void FHktMassNpcServerReplicationHelper::RemoveAgent(
	TArray<FHktReplicatedNpcAgentArrayItem>& Agents,
	FMassClientBubbleSerializerBase& Serializer,
	FMassReplicatedAgentHandle Handle)
{
	// Handle�?Agent 찾아???�거
	for (int32 i = 0; i < Agents.Num(); ++i)
	{
		if (Agents[i].GetHandle() == Handle)
		{
			Agents.RemoveAtSwap(i, 1);
			Serializer.MarkArrayDirty();
			return;
		}
	}
}

//----------------------------------------------------------------------//
// FHktMassNpcClientReplicationHelper
//----------------------------------------------------------------------//

void FHktMassNpcClientReplicationHelper::HandleItemAdded(
	const TArrayView<int32> AddedIndices,
	TArray<FHktReplicatedNpcAgentArrayItem>& Agents,
	FMassClientBubbleSerializerBase& Serializer)
{
#if UE_REPLICATION_COMPILE_CLIENT_CODE
	UWorld* World = Serializer.GetWorld();
	if (!World)
	{
		return;
	}

	FMassEntityManager& EntityManager = Serializer.GetEntityManagerChecked();
	UMassSpawnerSubsystem* SpawnerSubsystem = Serializer.GetSpawnerSubsystem();
	UMassReplicationSubsystem* ReplicationSubsystem = Serializer.GetReplicationSubsystem();
	
	if (!SpawnerSubsystem || !ReplicationSubsystem)
	{
		return;
	}

	// ?�플�?ID별로 그룹??
	TMap<FMassEntityTemplateID, TArray<int32>> AgentsByTemplate;
	
	for (int32 Idx : AddedIndices)
	{
		const FHktReplicatedNpcAgent& Agent = Agents[Idx].Agent;
		AgentsByTemplate.FindOrAdd(Agent.GetTemplateID()).Add(Idx);
	}

	// ?�플릿별�??�괄 ?�폰
	for (const auto& Pair : AgentsByTemplate)
	{
		const FMassEntityTemplateID& TemplateID = Pair.Key;
		const TArray<int32>& IndicesForTemplate = Pair.Value;

		TArray<FMassEntityHandle> SpawnedEntities;
		SpawnerSubsystem->SpawnEntities(TemplateID, IndicesForTemplate.Num(), FStructView(), TSubclassOf<UMassProcessor>(), SpawnedEntities);

		// ?�폰???�티??초기??
		for (int32 i = 0; i < IndicesForTemplate.Num(); ++i)
		{
			const int32 AgentIdx = IndicesForTemplate[i];
			const FHktReplicatedNpcAgent& Agent = Agents[AgentIdx].Agent;
			const FMassEntityHandle Entity = SpawnedEntities[i];

			// ReplicationSubsystem???�록
			ReplicationSubsystem->SetEntity(Agent.GetNetID(), Entity);

			// Fragment ?�데?�트
			FMassEntityView EntityView(EntityManager, Entity);
			UpdateEntityFragments(EntityView, Agent);
		}
	}
#endif // UE_REPLICATION_COMPILE_CLIENT_CODE
}

void FHktMassNpcClientReplicationHelper::HandleItemChanged(
	const TArrayView<int32> ChangedIndices,
	TArray<FHktReplicatedNpcAgentArrayItem>& Agents,
	FMassClientBubbleSerializerBase& Serializer)
{
#if UE_REPLICATION_COMPILE_CLIENT_CODE
	FMassEntityManager& EntityManager = Serializer.GetEntityManagerChecked();
	UMassReplicationSubsystem* ReplicationSubsystem = Serializer.GetReplicationSubsystem();
	
	if (!ReplicationSubsystem)
	{
		return;
	}

	// 변경된 ?�티?�의 Fragment ?�데?�트
	for (int32 Idx : ChangedIndices)
	{
		const FHktReplicatedNpcAgent& Agent = Agents[Idx].Agent;
		
		const FMassReplicationEntityInfo* EntityInfo = ReplicationSubsystem->FindMassEntityInfo(Agent.GetNetID());
		if (EntityInfo && EntityInfo->Entity.IsSet())
		{
			FMassEntityView EntityView(EntityManager, EntityInfo->Entity);
			UpdateEntityFragments(EntityView, Agent);
		}
	}
#endif // UE_REPLICATION_COMPILE_CLIENT_CODE
}

void FHktMassNpcClientReplicationHelper::HandleItemRemoved(
	const TArrayView<int32> RemovedIndices,
	TArray<FHktReplicatedNpcAgentArrayItem>& Agents,
	FMassClientBubbleSerializerBase& Serializer)
{
#if UE_REPLICATION_COMPILE_CLIENT_CODE
	UMassSpawnerSubsystem* SpawnerSubsystem = Serializer.GetSpawnerSubsystem();
	UMassReplicationSubsystem* ReplicationSubsystem = Serializer.GetReplicationSubsystem();
	
	if (!SpawnerSubsystem || !ReplicationSubsystem)
	{
		return;
	}

	TArray<FMassEntityHandle> EntitiesToDestroy;

	for (int32 Idx : RemovedIndices)
	{
		const FHktReplicatedNpcAgent& Agent = Agents[Idx].Agent;
		
		const FMassReplicationEntityInfo* EntityInfo = ReplicationSubsystem->FindMassEntityInfo(Agent.GetNetID());
		if (EntityInfo && EntityInfo->Entity.IsSet())
		{
			EntitiesToDestroy.Add(EntityInfo->Entity);
		}
	}

	// ?�티???�괄 ?�거
	if (EntitiesToDestroy.Num() > 0)
	{
		SpawnerSubsystem->DestroyEntities(EntitiesToDestroy);
	}
#endif // UE_REPLICATION_COMPILE_CLIENT_CODE
}

void FHktMassNpcClientReplicationHelper::UpdateEntityFragments(
	const FMassEntityView& EntityView,
	const FHktReplicatedNpcAgent& Agent)
{
#if UE_REPLICATION_COMPILE_CLIENT_CODE
	// Transform ?�데?�트
	if (FTransformFragment* TransformFragment = EntityView.GetFragmentDataPtr<FTransformFragment>())
	{
		TransformFragment->GetMutableTransform().SetLocation(Agent.Position);
		TransformFragment->GetMutableTransform().SetRotation(Agent.Rotation.Quaternion());
	}

	// Velocity ?�데?�트 (보간 ?�한 중요)
	if (FMassVelocityFragment* VelocityFragment = EntityView.GetFragmentDataPtr<FMassVelocityFragment>())
	{
		VelocityFragment->Value = Agent.Velocity;
	}

	// NPC Type ?�데?�트
	if (FHktNpcTypeFragment* TypeFragment = EntityView.GetFragmentDataPtr<FHktNpcTypeFragment>())
	{
		TypeFragment->NpcType = Agent.NpcType;
	}

	// State ?�데?�트
	if (FHktNpcStateFragment* StateFragment = EntityView.GetFragmentDataPtr<FHktNpcStateFragment>())
	{
		StateFragment->CurrentState = Agent.CurrentState;
	}

	// Combat (Health) ?�데?�트
	if (FHktNpcCombatFragment* CombatFragment = EntityView.GetFragmentDataPtr<FHktNpcCombatFragment>())
	{
		CombatFragment->CurrentHealth = Agent.CurrentHealth;
	}

	// ?�고: Animation??복제?��? ?�음
	// → HktMassNpcAnimationProcessor??Velocity?� State�??�고 ?�라?�언?�에???�체 계산
#endif // UE_REPLICATION_COMPILE_CLIENT_CODE
}

