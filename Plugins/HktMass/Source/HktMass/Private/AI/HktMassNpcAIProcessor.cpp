// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassNpcAIProcessor.h"
#include "HktMassCommonFragments.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassExecutionContext.h"

UHktMassNpcAIProcessor::UHktMassNpcAIProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone);
	
	// AI는 Behavior 그룹에서 실행 (Movement 전에)
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
}

void UHktMassNpcAIProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassDesiredMovementFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHktNpcStateFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHktNpcTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHktNpcCombatFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHktNpcPatrolFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FMassMovementParameters>(EMassFragmentPresence::All);
}

void UHktMassNpcAIProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMassVelocityFragment> VelocityList = Context.GetFragmentView<FMassVelocityFragment>();
		const TArrayView<FMassDesiredMovementFragment> DesiredMovementList = Context.GetMutableFragmentView<FMassDesiredMovementFragment>();
		const TArrayView<FHktNpcStateFragment> StateList = Context.GetMutableFragmentView<FHktNpcStateFragment>();
		const TArrayView<FHktNpcTargetFragment> TargetList = Context.GetMutableFragmentView<FHktNpcTargetFragment>();
		const TArrayView<FHktNpcCombatFragment> CombatList = Context.GetMutableFragmentView<FHktNpcCombatFragment>();
		const TConstArrayView<FHktNpcPatrolFragment> PatrolList = Context.GetFragmentView<FHktNpcPatrolFragment>();
		const FMassMovementParameters& MovementParams = Context.GetConstSharedFragment<FMassMovementParameters>();

		const float DeltaTime = Context.GetDeltaTimeSeconds();
		const float CurrentTime = Context.GetWorld()->GetTimeSeconds();

		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			const FTransformFragment& TransformFragment = TransformList[EntityIndex];
			FMassDesiredMovementFragment& DesiredMovement = DesiredMovementList[EntityIndex];
			FHktNpcStateFragment& StateFragment = StateList[EntityIndex];
			FHktNpcTargetFragment& TargetFragment = TargetList[EntityIndex];
			FHktNpcCombatFragment& CombatFragment = CombatList[EntityIndex];
			const FHktNpcPatrolFragment& PatrolFragment = PatrolList[EntityIndex];

			const FVector CurrentLocation = TransformFragment.GetTransform().GetLocation();

			// 체력??0 ?�하�??�망 처리
			if (CombatFragment.CurrentHealth <= 0.0f)
			{
				StateFragment.CurrentState = 4; // Dead
				continue;
			}

			// ?�태 ?�?�머 ?�데?�트
			StateFragment.StateTimer += DeltaTime;

			// ?�재 ?�태???�른 처리
			switch (StateFragment.CurrentState)
			{
				case 0: // Idle
				{
					// ?�정 ?�간 ???�찰 ?�태�??�환
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
					// ?�찰 경로가 ?�으�??�음 ?�인?�로 ?�동
					if (PatrolFragment.PatrolPoints.Num() > 0)
					{
						const int32 PatrolIndex = StateFragment.PatrolPointIndex % PatrolFragment.PatrolPoints.Num();
						const FVector PatrolPoint = PatrolFragment.PatrolPoints[PatrolIndex];

						TargetFragment.TargetLocation = PatrolPoint;
						TargetFragment.bHasValidTarget = true;

						// 목표 지?�에 ?�달?�는지 ?�인
						const float Distance = FVector::Distance(CurrentLocation, PatrolPoint);

						if (Distance < 50.0f)
						{
							// ?�음 ?�찰 ?�인?�로 ?�동
							StateFragment.PatrolPointIndex = (StateFragment.PatrolPointIndex + 1) % PatrolFragment.PatrolPoints.Num();
							StateFragment.StateTimer = 0.0f;

							// ?�시 ?��?							if (StateFragment.StateTimer > PatrolFragment.WaitTimeAtPoint)
							{
								StateFragment.StateTimer = 0.0f;
							}
						}
					}
					else
					{
						// ?�찰 경로가 ?�으�??�덤???�치�??�동
						if (StateFragment.StateTimer > 5.0f)
						{
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
					// ?��?추적 로직
					// ?�제로는 ?�른 ?�티?�나 ?�레?�어�?추적?�야 ?��?�?
					// ?�기?�는 ?�순?��? ?�해 ?�트�??�태�??�환
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
						// 공격 ?�행
						CombatFragment.LastAttackTime = CurrentTime;
						
						// ?�겟이 범위 밖이�?추적 ?�태�??�환
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

			// AI 결정에 따라 Desired Movement 설정
			// MassMovement 시스템이 이를 바탕으로 실제 움직임 생성
			if (StateFragment.CurrentState == 4) // Dead
			{
				// 사망 시 정지
				DesiredMovement.DesiredVelocity = FVector::ZeroVector;
				DesiredMovement.DesiredFacing = TransformFragment.GetTransform().GetRotation();
			}
			else if (TargetFragment.bHasValidTarget)
			{
				// 타겟이 있으면 그 방향으로 이동
				FVector Direction = (TargetFragment.TargetLocation - CurrentLocation);
				const float Distance = Direction.Length();
				TargetFragment.DistanceToTarget = Distance;

				if (Distance > 10.0f) // 목표에 충분히 가까우면 정지
				{
					Direction.Normalize();
					
					// Desired Velocity 설정 (MassMovement가 실제 이동 처리)
					DesiredMovement.DesiredVelocity = Direction * MovementParams.DefaultDesiredSpeed;
					DesiredMovement.DesiredFacing = Direction.ToOrientationQuat();
				}
				else
				{
					// 목표 도착, 정지
					DesiredMovement.DesiredVelocity = FVector::ZeroVector;
					DesiredMovement.DesiredFacing = TransformFragment.GetTransform().GetRotation();
				}
			}
			else
			{
				// 타겟 없으면 정지
				DesiredMovement.DesiredVelocity = FVector::ZeroVector;
				DesiredMovement.DesiredFacing = TransformFragment.GetTransform().GetRotation();
			}
		}
	});
}

void UHktMassNpcAIProcessor::ProcessIdleState(FMassExecutionContext& Context, int32 EntityIndex)
{
	// Idle ?�태 처리
}

void UHktMassNpcAIProcessor::ProcessPatrolState(FMassExecutionContext& Context, int32 EntityIndex)
{
	// Patrol ?�태 처리
}

void UHktMassNpcAIProcessor::ProcessChaseState(FMassExecutionContext& Context, int32 EntityIndex)
{
	// Chase ?�태 처리
}

void UHktMassNpcAIProcessor::ProcessAttackState(FMassExecutionContext& Context, int32 EntityIndex)
{
	// Attack ?�태 처리
}

//------------------------------------------------------------------------------
// UHktMassNpcPatrolProcessor
//------------------------------------------------------------------------------

UHktMassNpcPatrolProcessor::UHktMassNpcPatrolProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteInGroup = FName(TEXT("Tasks"));
}

void UHktMassNpcPatrolProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHktNpcPatrolFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHktNpcStateFragment>(EMassFragmentAccess::ReadOnly);
}

void UHktMassNpcPatrolProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// ?�찰 경로 ?�데?�트 로직
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FHktNpcPatrolFragment> PatrolList = Context.GetMutableFragmentView<FHktNpcPatrolFragment>();
		const TConstArrayView<FHktNpcStateFragment> StateList = Context.GetFragmentView<FHktNpcStateFragment>();

		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			// ?�찰 경로가 비어?�으�??�덤?�게 ?�성
			FHktNpcPatrolFragment& PatrolFragment = PatrolList[EntityIndex];
			
			if (PatrolFragment.PatrolPoints.Num() == 0)
			{
				const FTransformFragment& TransformFragment = TransformList[EntityIndex];
				const FVector StartLocation = TransformFragment.GetTransform().GetLocation();

				// 4개의 ?�덤 ?�찰 ?�인???�성
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

