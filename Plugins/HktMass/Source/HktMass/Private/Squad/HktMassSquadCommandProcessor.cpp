// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassSquadCommandProcessor.h"
#include "HktMassSquadFragments.h"
#include "HktMassDefines.h"
#include "HktMassMovementFragments.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"
#include "Engine/World.h"

UHktMassSquadCommandProcessor::UHktMassSquadCommandProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
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
	// Subsystem 의존성 제거
	
	EntityQuery.ForEachEntityChunk(ExecutionContext, [&EntityManager](FMassExecutionContext& Context)
	{
		const TConstArrayView<FHktMassSquadMemberFragment> SquadMembers = Context.GetFragmentView<FHktMassSquadMemberFragment>();
		const TArrayView<FHktMassMoveToLocationFragment> MoveToFragments = Context.GetMutableFragmentView<FHktMassMoveToLocationFragment>();
		const TArrayView<FTransformFragment> TransFragments = Context.GetMutableFragmentView<FTransformFragment>();

		for (int32 i = 0; i < Context.GetNumEntities(); ++i)
		{
			// Fragment에 저장된 Parent Handle 직접 사용
			const FMassEntityHandle SquadHandle = SquadMembers[i].ParentSquadEntity;
			if (SquadHandle.IsSet())
			{
				// 분대 Entity의 위치 가져오기
				if (const FTransformFragment* SquadTransformFrag = EntityManager.GetFragmentDataPtr<FTransformFragment>(SquadHandle))
				{
					const FVector LeaderPos = SquadTransformFrag->GetTransform().GetLocation();
					
					// FormationOffset을 더해 목표 위치 설정
					FVector TargetPos = LeaderPos + SquadMembers[i].FormationOffset;

					MoveToFragments[i].TargetLocation = TargetPos;
				}
			}
		}
	});
}
