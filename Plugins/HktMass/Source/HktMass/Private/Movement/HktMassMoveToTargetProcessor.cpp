// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassMoveToTargetProcessor.h"
#include "HktMassMovementFragments.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "Engine/World.h"
#include "MassExecutionContext.h"
#include "MassSimulationLOD.h"

UHktMassMoveToTargetProcessor::UHktMassMoveToTargetProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	// 이동 로직은 주로 PrePhysics나 Movement 단계에서 실행합니다.
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement; 
}

void UHktMassMoveToTargetProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// 1. 목표 위치는 읽기 전용으로 가져옵니다.
	EntityQuery.AddRequirement<FHktMassMoveToLocationFragment>(EMassFragmentAccess::ReadOnly);
	
	// 2. 실제 이동을 반영할 Transform은 읽기/쓰기 권한이 필요합니다.
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	
	// 3. 현재 속도(Velocity)도 업데이트하여 애니메이션 등에 활용할 수 있도록 합니다.
	EntityQuery.AddRequirement<FHktMassVelocityFragment>(EMassFragmentAccess::ReadWrite);

	// 5. LOD 처리를 위한 Chunk Requirement (기존 스타일 유지)
	EntityQuery.AddRequirement<FMassSimulationVariableTickFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.SetChunkFilter(&FMassSimulationVariableTickChunkFragment::ShouldTickChunkThisFrame);
}

void UHktMassMoveToTargetProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& ExecutionContext)
{
	// 이동 속도 상수 (실제로는 Fragment에 Speed 속성을 추가하여 가져오는 것이 좋습니다)
	const double MoveSpeed = 500.0; 
	const double AcceptanceRadius = 10.0; // 목표 도달 허용 오차
	const double AcceptanceRadiusSq = AcceptanceRadius * AcceptanceRadius;

	EntityQuery.ForEachEntityChunk(ExecutionContext, ([this, MoveSpeed, AcceptanceRadiusSq](FMassExecutionContext& Context)
	{
		const TConstArrayView<FHktMassMoveToLocationFragment> TargetsList = Context.GetFragmentView<FHktMassMoveToLocationFragment>();
		const TArrayView<FTransformFragment> TransformsList = Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FHktMassVelocityFragment> VelocitiesList = Context.GetMutableFragmentView<FHktMassVelocityFragment>();
		
		const TConstArrayView<FMassSimulationVariableTickFragment> SimVariableTickList = Context.GetFragmentView<FMassSimulationVariableTickFragment>();
		const bool bHasVariableTick = (SimVariableTickList.Num() > 0);
		const float WorldDeltaTime = Context.GetDeltaTimeSeconds();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FVector TargetLoc = TargetsList[EntityIt].TargetLocation;
			FTransform& Transform = TransformsList[EntityIt].GetMutableTransform();
			FVector& CurrentVelocity = VelocitiesList[EntityIt].Value;

			const FVector CurrentLoc = Transform.GetLocation();
			FVector ToTarget = TargetLoc - CurrentLoc;
			const double DistSq = ToTarget.SizeSquared();

			// LOD에 따른 DeltaTime 계산
			const float DeltaTime = bHasVariableTick ? SimVariableTickList[EntityIt].DeltaTime : WorldDeltaTime;

			// 목표에 도달하지 않았을 경우 이동
			if (DistSq > AcceptanceRadiusSq)
			{
				// 방향 벡터 정규화
				const FVector Direction = ToTarget.GetSafeNormal();
				
				// 속도 갱신 (방향 * 속력)
				CurrentVelocity = Direction * MoveSpeed;

				// 위치 이동 (P = P0 + V * dt)
				// 만약 남은 거리가 (Speed * DeltaTime)보다 작다면 바로 목표 위치로 스냅해주는 로직을 추가할 수도 있습니다.
				const FVector MoveDelta = CurrentVelocity * DeltaTime;
				
				// 과도하게 지나치지 않도록 처리 (Simple check)
				if (MoveDelta.SizeSquared() > DistSq)
				{
					Transform.SetLocation(TargetLoc);
					CurrentVelocity = FVector::ZeroVector;
				}
				else
				{
					Transform.AddToTranslation(MoveDelta);
					
					// (선택 사항) 이동 방향을 보게 회전
					if (!Direction.IsNearlyZero())
					{
						Transform.SetRotation(Direction.ToOrientationQuat());
					}
				}
			}
			else
			{
				// 목표 도달 시 정지
				CurrentVelocity = FVector::ZeroVector;
				// 필요하다면 정확한 위치로 강제 설정
				// Transform.SetLocation(TargetLoc); 
			}
		}
	}));
}