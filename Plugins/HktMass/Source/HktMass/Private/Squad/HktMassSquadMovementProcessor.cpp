// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassSquadMovementProcessor.h"
#include "HktMassSquadFragments.h"
#include "HktMassSquadSubsystem.h"
#include "HktMassSquadLeader.h"
#include "HktMassMovementFragments.h"
#include "HktMassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"
#include "Engine/World.h"

UHktMassSquadMovementProcessor::UHktMassSquadMovementProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	// AI 판단(Behavior) 단계에서 실행
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
}

void UHktMassSquadMovementProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// 1. 분대 정보 읽기
	EntityQuery.AddRequirement<FHktMassSquadMemberFragment>(EMassFragmentAccess::ReadOnly);
	// 2. 목표 위치 쓰기
	EntityQuery.AddRequirement<FHktMassMoveToLocationFragment>(EMassFragmentAccess::ReadWrite);
}

void UHktMassSquadMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& ExecutionContext)
{
	UWorld* World = EntityManager.GetWorld();
	if (!World) return;

	UHktMassSquadSubsystem* SquadSubsystem = World->GetSubsystem<UHktMassSquadSubsystem>();
	if (!SquadSubsystem) return;

	EntityQuery.ForEachEntityChunk(ExecutionContext, [SquadSubsystem](FMassExecutionContext& Context)
	{
		const TConstArrayView<FHktMassSquadMemberFragment> SquadMembers = Context.GetFragmentView<FHktMassSquadMemberFragment>();
		const TArrayView<FHktMassMoveToLocationFragment> MoveToFragments = Context.GetMutableFragmentView<FHktMassMoveToLocationFragment>();

		for (int32 i = 0; i < Context.GetNumEntities(); ++i)
		{
			const int32 MySquadID = SquadMembers[i].SquadID;
			
			// 서브시스템에서 캐싱된 리더 액터를 찾음 (비용 저렴)
			if (AHktMassSquadLeader* Leader = SquadSubsystem->GetSquadLeader(MySquadID))
			{
				// 리더의 위치를 목표로 설정
				FVector LeaderPos = Leader->GetActorLocation();
				
				// FormationOffset을 더해 겹침 방지 (실제로는 원형 진형 등을 계산)
				// 간단하게 랜덤 오프셋이나 고정 오프셋을 적용
				FVector TargetPos = LeaderPos + SquadMembers[i].FormationOffset;

				MoveToFragments[i].TargetLocation = TargetPos;
			}
		}
	});
}