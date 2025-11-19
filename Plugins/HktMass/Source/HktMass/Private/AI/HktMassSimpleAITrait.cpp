// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassSimpleAITrait.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"

void UHktMassSimpleAITrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	// 이동 시스템 의존성 추가
	BuildContext.RequireFragment<FHktMassMoveToLocationFragment>();
	BuildContext.RequireFragment<FHktMassVelocityFragment>();
	BuildContext.RequireFragment<FTransformFragment>();

	BuildContext.AddTag<FHktMassSimpleAITag>();

	// 초기 데이터 설정
	// 주의: BuildTemplate은 CDO 생성 시점이나 MassSpawner 초기화 시점에 호출됩니다.
	if (!PatrolActorTag.IsNone())
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsWithTag(&World, PatrolActorTag, FoundActors);

		if (FoundActors.Num() > 0)
		{
			FHktMassPatrolFragment& PatrolFrag = BuildContext.AddFragment_GetRef<FHktMassPatrolFragment>();

			PatrolFrag.Waypoints.Reset();
			for (AActor* Actor : FoundActors)
			{
				if (Actor)
				{
					PatrolFrag.Waypoints.Add(Actor->GetActorLocation());
				}
			}

			// 액터 순서가 무작위일 수 있으므로 이름순 정렬 등 필요시 추가 로직 구현 가능
			// 여기서는 발견된 순서대로 저장합니다.

			// 첫 번째 목표 지점 설정
			if (PatrolFrag.Waypoints.Num() > 0)
			{
				PatrolFrag.CurrentWaypointIndex = 0;
			}
		}
	}
}
