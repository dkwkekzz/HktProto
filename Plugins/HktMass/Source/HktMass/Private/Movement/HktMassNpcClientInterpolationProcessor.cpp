// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassNpcClientInterpolationProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassExecutionContext.h"
#include "MassSimulationLOD.h"

UHktMassNpcClientInterpolationProcessor::UHktMassNpcClientInterpolationProcessor()
	: EntityQuery(*this)
{
	// 클라이언트 전용 실행
	ExecutionFlags = (int32)EProcessorExecutionFlags::Client;
	
	// Movement 그룹에서 실행
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
	
	// 리플리케이션 후에 실행 (서버 데이터 받은 후)
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::SyncWorldToMass);
}

void UHktMassNpcClientInterpolationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	
	// Off-LOD 엔티티는 처리하지 않음
	EntityQuery.AddTagRequirement<FMassOffLODTag>(EMassFragmentPresence::None);
}

void UHktMassNpcClientInterpolationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// 델타 타임 클램핑 (큰 시간 점프 방지)
	const float DeltaTime = FMath::Min(0.1f, Context.GetDeltaTimeSeconds());
	
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [DeltaTime](FMassExecutionContext& Context)
	{
		const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
		const TConstArrayView<FMassVelocityFragment> VelocityList = Context.GetFragmentView<FMassVelocityFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		
		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			FTransformFragment& TransformFragment = TransformList[EntityIndex];
			const FMassVelocityFragment& VelocityFragment = VelocityList[EntityIndex];
			
			// Velocity로 선형 보간
			FTransform& Transform = TransformFragment.GetMutableTransform();
			FVector CurrentLocation = Transform.GetLocation();
			FVector NewLocation = CurrentLocation + VelocityFragment.Value * DeltaTime;
			
			Transform.SetTranslation(NewLocation);
			
			// Velocity 방향으로 회전 (속도가 충분히 클 때만)
			if (VelocityFragment.Value.SizeSquared() > 100.0f) // 10 cm/s 이상
			{
				FRotator NewRotation = VelocityFragment.Value.Rotation();
				NewRotation.Pitch = 0.0f;
				NewRotation.Roll = 0.0f;
				Transform.SetRotation(NewRotation.Quaternion());
			}
		}
	});
}

