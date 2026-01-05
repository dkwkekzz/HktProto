// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "Flows/HktMoveToLocationBehavior.h"
#include "HktFlowBuilder.h"

FGameplayTag UHktMoveToLocationBehavior::GetStaticEventTag()
{
	return FGameplayTag::RequestGameplayTag(FName("Event.Movement.MoveTo"));
}

FGameplayTag UHktMoveToLocationBehavior::GetEventTag() const
{
	return GetStaticEventTag();
}

void UHktMoveToLocationBehavior::DefineFlow(FHktFlowBuilder& Flow, const void* EventData)
{
	const FHktMoveToLocationEventData* Data = static_cast<const FHktMoveToLocationEventData*>(EventData);
	if (!Data || Data->SubjectHandle == INDEX_NONE)
	{
		return;
	}

	const int32 Subject = Data->SubjectHandle;
	const float Mass = Data->Mass;

	/*
	 * 이동 흐름:
	 * 
	 * 1. 목표 위치를 향해 이동
	 * 2. 도착 → 정지
	 * 3. 충돌 → 질량에 따른 반발력 적용
	 *    - 무거우면 밀어냄
	 *    - 가벼우면 튕겨남
	 */

	Flow.MoveTo(Subject, Data->TargetLocation)
		.WithFloat(Data->SpeedMultiplier)
		
		// 목표에 도착하면
		.OnReachLocation([&, Subject]()
		{
			Flow.Stop(Subject);
		})
		
		// 충돌하면
		.OnCollision([&, Subject, Mass](int32 CollidedWith)
		{
			// 충돌 상대의 질량과 비교하여 힘 방향 결정
			// 여기서는 CollidedWith를 통해 상대 정보 조회 가능
			// 힘의 크기는 질량 비율에 반비례
			
			// 1. 이동 멈춤
			Flow.Stop(Subject);
			
			// 2. 충돌 반발력 적용 (방향은 런타임에 결정)
			//    Force vector는 실제로는 충돌 법선에서 계산됨
			//    여기서는 "반발력을 받는다"는 개념만 표현
			Flow.ApplyForce(Subject, FVector::ZeroVector) // 방향은 런타임 결정
				.WithFloat(1.0f / Mass) // 질량이 클수록 적은 힘
				.WithInt(CollidedWith); // 충돌 상대 정보 전달
		});
}

