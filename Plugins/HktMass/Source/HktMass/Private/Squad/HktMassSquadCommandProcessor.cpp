// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassSquadCommandProcessor.h"
#include "HktMassSquadFragments.h"
#include "HktMassSquadSubsystem.h"
#include "HktMassSquadCommandComponent.h"
#include "HktMassDefines.h"
#include "HktMassMovementFragments.h"
#include "HktMassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"
#include "Engine/World.h"

UHktMassSquadCommandProcessor::UHktMassSquadCommandProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	// Squad 그룹에서 실행
	ExecutionOrder.ExecuteInGroup = HktMass::ExecuteGroupNames::Squad;
}

void UHktMassSquadCommandProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// 1. 분대 정보 읽기
	EntityQuery.AddRequirement<FHktMassSquadMemberFragment>(EMassFragmentAccess::ReadOnly);
	// 2. 목표 위치 쓰기
	EntityQuery.AddRequirement<FHktMassMoveToLocationFragment>(EMassFragmentAccess::ReadWrite);
}

void UHktMassSquadCommandProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& ExecutionContext)
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
			
			// 서브시스템에서 캐싱된 분대 커맨드 컴포넌트를 찾음 (비용 저렴)
			if (UHktMassSquadCommandComponent* CommandComponent = SquadSubsystem->GetSquadCommandComponent(MySquadID))
			{
				// 컴포넌트에 캐싱된 위치를 목표로 설정
				const FVector LeaderPos = CommandComponent->GetSquadLocation();
				
				// FormationOffset을 더해 겹침 방지 (실제로는 원형 진형 등을 계산)
				// 간단하게 랜덤 오프셋이나 고정 오프셋을 적용
				FVector TargetPos = LeaderPos + SquadMembers[i].FormationOffset;

				MoveToFragments[i].TargetLocation = TargetPos;
			}
		}
	});
}