// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassSimpleAIProcessor.h"
#include "HktMassAIFragments.h"
#include "HktMassMovementFragments.h"
#include "HktMassDefines.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"

// ===================================
// UHktMassSimpleAIFragmentInitializer
// ===================================

UHktMassSimpleAIFragmentInitializer::UHktMassSimpleAIFragmentInitializer()
	: EntityQuery(*this)
{
	ObservedType = FHktMassPatrolFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;

	ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteInGroup = HktMass::ExecuteGroupNames::AI;
}

void UHktMassSimpleAIFragmentInitializer::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FHktMassPatrolFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHktMassMoveToLocationFragment>(EMassFragmentAccess::ReadWrite);
}

void UHktMassSimpleAIFragmentInitializer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& ExecutionContext)
{
	EntityQuery.ForEachEntityChunk(ExecutionContext, [](FMassExecutionContext& Context)
	{
		TArrayView<FHktMassPatrolFragment> PatrolList = Context.GetMutableFragmentView<FHktMassPatrolFragment>();
		TArrayView<FHktMassMoveToLocationFragment> MoveToList = Context.GetMutableFragmentView<FHktMassMoveToLocationFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FHktMassPatrolFragment& PatrolFrag = PatrolList[EntityIt];
			FHktMassMoveToLocationFragment& MoveToFrag = MoveToList[EntityIt];

			PatrolFrag.CurrentWaypointIndex = 0;
			MoveToFrag.TargetLocation = PatrolFrag.Waypoints[PatrolFrag.CurrentWaypointIndex];
		}
	});
}

// ===================================
// UHktMassNpcAnimationProcessor
// ===================================

UHktMassSimpleAIProcessor::UHktMassSimpleAIProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;

	ExecutionOrder.ExecuteInGroup = HktMass::ExecuteGroupNames::AI;
}

void UHktMassSimpleAIProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// 1. 순찰 정보 (읽기/쓰기: 인덱스 업데이트)
	EntityQuery.AddRequirement<FHktMassPatrolFragment>(EMassFragmentAccess::ReadWrite);
	
	// 2. 이동 명령 (읽기/쓰기: 목표 위치 설정)
	EntityQuery.AddRequirement<FHktMassMoveToLocationFragment>(EMassFragmentAccess::ReadWrite);
	
	// 3. 현재 위치 확인용 (읽기 전용)
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);

	// 4. 태그 필터
	EntityQuery.AddTagRequirement<FHktMassSimpleAITag>(EMassFragmentPresence::All);
}

void UHktMassSimpleAIProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& ExecutionContext)
{
	EntityQuery.ForEachEntityChunk(ExecutionContext, [](FMassExecutionContext& Context)
	{
		const TArrayView<FHktMassPatrolFragment> PatrolList = Context.GetMutableFragmentView<FHktMassPatrolFragment>();
		const TArrayView<FHktMassMoveToLocationFragment> MoveToList = Context.GetMutableFragmentView<FHktMassMoveToLocationFragment>();
		const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FHktMassPatrolFragment& PatrolFrag = PatrolList[EntityIndex];
			FHktMassMoveToLocationFragment& MoveToFrag = MoveToList[EntityIndex];
			const FTransform& Transform = TransformList[EntityIndex].GetTransform();

			if (PatrolFrag.Waypoints.Num() == 0)
			{
				continue;
			}

			// 현재 목표 지점 가져오기
			const FVector CurrentTarget = PatrolFrag.Waypoints[PatrolFrag.CurrentWaypointIndex];
			
			// 목표가 제대로 설정되어 있는지 확인 (초기화 동기화)
			MoveToFrag.TargetLocation = CurrentTarget;

			// 거리 체크
			const float DistSq = FVector::DistSquared(Transform.GetLocation(), CurrentTarget);
			const float RadiusSq = PatrolFrag.AcceptanceRadius * PatrolFrag.AcceptanceRadius;

			// 도착 판정
			if (DistSq <= RadiusSq)
			{
				// 다음 인덱스로 변경 (Loop)
				PatrolFrag.CurrentWaypointIndex = (PatrolFrag.CurrentWaypointIndex + 1) % PatrolFrag.Waypoints.Num();
				
				// 이동 명령 업데이트
				MoveToFrag.TargetLocation = PatrolFrag.Waypoints[PatrolFrag.CurrentWaypointIndex];
			}
		}
	});
}