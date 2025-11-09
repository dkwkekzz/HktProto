// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassNpcMovementProcessor.h"
#include "HktMassNpcFragments.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"

UHktMassNpcMovementProcessor::UHktMassNpcMovementProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteInGroup = FName(TEXT("Movement"));
}

void UHktMassNpcMovementProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHktNpcMovementFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHktNpcTargetFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHktNpcStateFragment>(EMassFragmentAccess::ReadOnly);
}

void UHktMassNpcMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
		const TConstArrayView<FHktNpcMovementFragment> MovementList = Context.GetFragmentView<FHktNpcMovementFragment>();
		const TConstArrayView<FHktNpcTargetFragment> TargetList = Context.GetFragmentView<FHktNpcTargetFragment>();
		const TConstArrayView<FHktNpcStateFragment> StateList = Context.GetFragmentView<FHktNpcStateFragment>();

		const float DeltaTime = Context.GetDeltaTimeSeconds();

		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			FTransformFragment& TransformFragment = TransformList[EntityIndex];
			const FHktNpcMovementFragment& MovementFragment = MovementList[EntityIndex];
			const FHktNpcTargetFragment& TargetFragment = TargetList[EntityIndex];
			const FHktNpcStateFragment& StateFragment = StateList[EntityIndex];

			// Dead 상태면 이동하지 않음
			if (StateFragment.CurrentState == 4)
			{
				continue;
			}

			FTransform& Transform = TransformFragment.GetMutableTransform();
			FVector CurrentLocation = Transform.GetLocation();

			// 타겟이 있으면 타겟 방향으로 이동
			if (TargetFragment.bHasValidTarget)
			{
				FVector Direction = (TargetFragment.TargetLocation - CurrentLocation);
				const float Distance = Direction.Length();

				if (Distance > 10.0f) // 목표 지점에 거의 도달하지 않았으면
				{
					Direction.Normalize();
					
					// 새로운 위치 계산
					FVector NewLocation = CurrentLocation + Direction * MovementFragment.CurrentSpeed * DeltaTime;
					
					// 회전도 업데이트
					FRotator NewRotation = Direction.Rotation();
					NewRotation.Pitch = 0.0f; // 지면과 평행하게
					NewRotation.Roll = 0.0f;

					Transform.SetLocation(NewLocation);
					Transform.SetRotation(NewRotation.Quaternion());
				}
			}
		}
	});
}

