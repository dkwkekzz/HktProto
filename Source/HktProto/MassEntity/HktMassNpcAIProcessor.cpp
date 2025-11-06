// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassNpcAIProcessor.h"
#include "HktMassNpcFragments.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"

UHktMassNpcAIProcessor::UHktMassNpcAIProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteInGroup = FName(TEXT("Tasks"));
	ExecutionOrder.ExecuteAfter.Add(FName(TEXT("Movement")));
}

void UHktMassNpcAIProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
}

void UHktMassNpcAIProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	FMassEntityQuery EntityQuery;
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHktNpcStateFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHktNpcTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHktNpcCombatFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHktNpcPatrolFragment>(EMassFragmentAccess::ReadOnly);

	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FHktNpcStateFragment> StateList = Context.GetMutableFragmentView<FHktNpcStateFragment>();
		const TArrayView<FHktNpcTargetFragment> TargetList = Context.GetMutableFragmentView<FHktNpcTargetFragment>();
		const TArrayView<FHktNpcCombatFragment> CombatList = Context.GetMutableFragmentView<FHktNpcCombatFragment>();
		const TConstArrayView<FHktNpcPatrolFragment> PatrolList = Context.GetFragmentView<FHktNpcPatrolFragment>();

		const float DeltaTime = Context.GetDeltaTimeSeconds();
		const float CurrentTime = Context.GetWorld()->GetTimeSeconds();

		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			const FTransformFragment& TransformFragment = TransformList[EntityIndex];
			FHktNpcStateFragment& StateFragment = StateList[EntityIndex];
			FHktNpcTargetFragment& TargetFragment = TargetList[EntityIndex];
			FHktNpcCombatFragment& CombatFragment = CombatList[EntityIndex];
			const FHktNpcPatrolFragment& PatrolFragment = PatrolList[EntityIndex];

			// 체력이 0 이하면 사망 처리
			if (CombatFragment.CurrentHealth <= 0.0f)
			{
				StateFragment.CurrentState = 4; // Dead
				continue;
			}

			// 상태 타이머 업데이트
			StateFragment.StateTimer += DeltaTime;

			// 현재 상태에 따른 처리
			switch (StateFragment.CurrentState)
			{
				case 0: // Idle
				{
					// 일정 시간 후 순찰 상태로 전환
					if (StateFragment.StateTimer > 2.0f)
					{
						StateFragment.CurrentState = 1; // Patrol
						StateFragment.StateTimer = 0.0f;
						StateFragment.PatrolPointIndex = 0;
					}
					break;
				}

				case 1: // Patrol
				{
					// 순찰 경로가 있으면 다음 포인트로 이동
					if (PatrolFragment.PatrolPoints.Num() > 0)
					{
						const int32 PatrolIndex = StateFragment.PatrolPointIndex % PatrolFragment.PatrolPoints.Num();
						const FVector PatrolPoint = PatrolFragment.PatrolPoints[PatrolIndex];

						TargetFragment.TargetLocation = PatrolPoint;
						TargetFragment.bHasValidTarget = true;

						// 목표 지점에 도달했는지 확인
						const FVector CurrentLocation = TransformFragment.GetTransform().GetLocation();
						const float Distance = FVector::Distance(CurrentLocation, PatrolPoint);

						if (Distance < 50.0f)
						{
							// 다음 순찰 포인트로 이동
							StateFragment.PatrolPointIndex = (StateFragment.PatrolPointIndex + 1) % PatrolFragment.PatrolPoints.Num();
							StateFragment.StateTimer = 0.0f;

							// 잠시 대기
							if (StateFragment.StateTimer > PatrolFragment.WaitTimeAtPoint)
							{
								StateFragment.StateTimer = 0.0f;
							}
						}
					}
					else
					{
						// 순찰 경로가 없으면 랜덤한 위치로 이동
						if (StateFragment.StateTimer > 5.0f)
						{
							const FVector CurrentLocation = TransformFragment.GetTransform().GetLocation();
							const FVector RandomOffset = FVector(
								FMath::FRandRange(-PatrolFragment.PatrolRadius, PatrolFragment.PatrolRadius),
								FMath::FRandRange(-PatrolFragment.PatrolRadius, PatrolFragment.PatrolRadius),
								0.0f
							);

							TargetFragment.TargetLocation = CurrentLocation + RandomOffset;
							TargetFragment.bHasValidTarget = true;
							StateFragment.StateTimer = 0.0f;
						}
					}
					break;
				}

				case 2: // Chase
				{
					// 타겟 추적 로직
					// 실제로는 다른 엔티티나 플레이어를 추적해야 하지만,
					// 여기서는 단순화를 위해 패트롤 상태로 전환
					if (StateFragment.StateTimer > 5.0f)
					{
						StateFragment.CurrentState = 1; // Patrol
						StateFragment.StateTimer = 0.0f;
					}
					break;
				}

				case 3: // Attack
				{
					// 공격 로직
					if (CurrentTime - CombatFragment.LastAttackTime >= CombatFragment.AttackCooldown)
					{
						// 공격 수행
						CombatFragment.LastAttackTime = CurrentTime;
						
						// 타겟이 범위 밖이면 추적 상태로 전환
						if (TargetFragment.DistanceToTarget > CombatFragment.AttackRange)
						{
							StateFragment.CurrentState = 2; // Chase
							StateFragment.StateTimer = 0.0f;
						}
					}
					break;
				}

				default:
					break;
			}
		}
	});
}

void UHktMassNpcAIProcessor::ProcessIdleState(FMassExecutionContext& Context, int32 EntityIndex)
{
	// Idle 상태 처리
}

void UHktMassNpcAIProcessor::ProcessPatrolState(FMassExecutionContext& Context, int32 EntityIndex)
{
	// Patrol 상태 처리
}

void UHktMassNpcAIProcessor::ProcessChaseState(FMassExecutionContext& Context, int32 EntityIndex)
{
	// Chase 상태 처리
}

void UHktMassNpcAIProcessor::ProcessAttackState(FMassExecutionContext& Context, int32 EntityIndex)
{
	// Attack 상태 처리
}

//------------------------------------------------------------------------------
// UHktMassNpcPatrolProcessor
//------------------------------------------------------------------------------

UHktMassNpcPatrolProcessor::UHktMassNpcPatrolProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteInGroup = FName(TEXT("Tasks"));
}

void UHktMassNpcPatrolProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
}

void UHktMassNpcPatrolProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	FMassEntityQuery EntityQuery;
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHktNpcPatrolFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHktNpcStateFragment>(EMassFragmentAccess::ReadOnly);

	// 순찰 경로 업데이트 로직
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FHktNpcPatrolFragment> PatrolList = Context.GetMutableFragmentView<FHktNpcPatrolFragment>();
		const TConstArrayView<FHktNpcStateFragment> StateList = Context.GetFragmentView<FHktNpcStateFragment>();

		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			// 순찰 경로가 비어있으면 랜덤하게 생성
			FHktNpcPatrolFragment& PatrolFragment = PatrolList[EntityIndex];
			
			if (PatrolFragment.PatrolPoints.Num() == 0)
			{
				const FTransformFragment& TransformFragment = TransformList[EntityIndex];
				const FVector StartLocation = TransformFragment.GetTransform().GetLocation();

				// 4개의 랜덤 순찰 포인트 생성
				for (int32 i = 0; i < 4; ++i)
				{
					const FVector RandomOffset = FVector(
						FMath::FRandRange(-PatrolFragment.PatrolRadius, PatrolFragment.PatrolRadius),
						FMath::FRandRange(-PatrolFragment.PatrolRadius, PatrolFragment.PatrolRadius),
						0.0f
					);

					PatrolFragment.PatrolPoints.Add(StartLocation + RandomOffset);
				}
			}
		}
	});
}

