// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassNpcVisualizationProcessor.h"
#include "HktMassNpcManagerSubsystem.h"
#include "HktMassNpcFragments.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"

UHktMassNpcVisualizationProcessor::UHktMassNpcVisualizationProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteInGroup = FName(TEXT("Representation"));
	ExecutionOrder.ExecuteAfter.Add(FName(TEXT("Movement")));
}

void UHktMassNpcVisualizationProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
    Super::InitializeInternal(Owner, EntityManager);
	NpcManagerSubsystem = UWorld::GetSubsystem<UHktMassNpcManagerSubsystem>(Owner.GetWorld());
}

void UHktMassNpcVisualizationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHktNpcVisualizationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHktNpcTypeFragment>(EMassFragmentAccess::ReadOnly);
}

void UHktMassNpcVisualizationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// 시각화 업데이트 로직
	// 실제로는 Instanced Static Mesh를 업데이트하거나
	// LOD 시스템과 통합하여 처리해야 합니다.
	
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FHktNpcVisualizationFragment> VisualizationList = Context.GetMutableFragmentView<FHktNpcVisualizationFragment>();
		const TConstArrayView<FHktNpcTypeFragment> TypeList = Context.GetFragmentView<FHktNpcTypeFragment>();

		if (!NpcManagerSubsystem)
		{
			return;
		}

		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			const FTransformFragment& TransformFragment = TransformList[EntityIndex];
			FHktNpcVisualizationFragment& VisualizationFragment = VisualizationList[EntityIndex];
			const FHktNpcTypeFragment& TypeFragment = TypeList[EntityIndex];

			// Transform 업데이트
			const FTransform& EntityTransform = TransformFragment.GetTransform();

			// ISM 인스턴스가 없으면 생성
			if (VisualizationFragment.InstanceIndex == -1)
			{
				// 여기서 실제로 ISM 인스턴스를 생성해야 합니다.
				// 이는 별도의 관리 시스템이나 Spawner에서 처리하는 것이 좋습니다.
			}
			else
			{
				// ISM 인스턴스 업데이트
				// 실제 구현 시 적절한 ISM 컴포넌트를 선택하여 업데이트
				UInstancedStaticMeshComponent* TargetISM = nullptr;

				switch(TypeFragment.NpcType)
				{
					case 0: // Melee
						TargetISM = NpcManagerSubsystem->GetMeleeNpcMeshISM();
						break;
					case 1: // Ranged
						TargetISM = NpcManagerSubsystem->GetRangedNpcMeshISM();
						break;
					case 2: // Tank
						TargetISM = NpcManagerSubsystem->GetTankNpcMeshISM();
						break;
				}

				if (TargetISM && VisualizationFragment.InstanceIndex >= 0)
				{
					TargetISM->UpdateInstanceTransform(
						VisualizationFragment.InstanceIndex,
						EntityTransform,
						true,
						false,
						true
					);
				}
			}
		}
	});
}

