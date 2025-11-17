// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassNpcUpdateISMBoneAnimationProcessor.h"
#include "HktMassNpcAnimationTypes.h"
#include "MassVisualizationComponent.h"
#include "MassRepresentationSubsystem.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "MassRepresentationFragments.h"
#include "MassCommonFragments.h"
#include "MassLODFragments.h"
#include "MassUpdateISMProcessor.h"
#include "AnimToTextureDataAsset.h"
#include "AnimToTextureInstancePlaybackHelpers.h"

UHktMassNpcUpdateISMBoneAnimationProcessor::UHktMassNpcUpdateISMBoneAnimationProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);

	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Representation);
	bRequiresGameThreadExecution = true;
}

void UHktMassNpcUpdateISMBoneAnimationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSharedRequirement<FMassRepresentationSubsystemSharedFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHktMassNpcAnimationFragment>(EMassFragmentAccess::ReadOnly);
}

void UHktMassNpcUpdateISMBoneAnimationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		// Get representation subsystem
		UMassRepresentationSubsystem* RepresentationSubsystem = Context.GetSharedFragment<FMassRepresentationSubsystemSharedFragment>().RepresentationSubsystem;
		check(RepresentationSubsystem);
		FMassInstancedStaticMeshInfoArrayView ISMInfo = RepresentationSubsystem->GetMutableInstancedStaticMeshInfos();

		// Get fragment views
		TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
		TArrayView<FMassRepresentationFragment> RepresentationList = Context.GetMutableFragmentView<FMassRepresentationFragment>();
		TConstArrayView<FMassRepresentationLODFragment> RepresentationLODList = Context.GetFragmentView<FMassRepresentationLODFragment>();
		TConstArrayView<FHktMassNpcAnimationFragment> AnimationDataList = Context.GetFragmentView<FHktMassNpcAnimationFragment>();

		// Iterate through all entities in this chunk
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FTransformFragment& TransformFragment = TransformList[EntityIt];
			FMassRepresentationFragment& Representation = RepresentationList[EntityIt];
			const FMassRepresentationLODFragment& RepresentationLOD = RepresentationLODList[EntityIt];
			const FHktMassNpcAnimationFragment& AnimationData = AnimationDataList[EntityIt];

			// Only update if currently represented as static mesh instance
			if (Representation.CurrentRepresentation == EMassRepresentationType::StaticMeshInstance)
			{
				const int32 DescIndex = Representation.StaticMeshDescHandle.ToIndex();

				// Update Transform (inherited from UMassUpdateISMProcessor)
				UMassUpdateISMProcessor::UpdateISMTransform(
					Context.GetEntity(EntityIt),
					ISMInfo[DescIndex],
					TransformFragment.GetTransform(),
					Representation.PrevTransform,
					RepresentationLOD.LODSignificance,
					Representation.PrevLODSignificance);

				// Update bone animation CustomData
				UpdateISMBoneAnimation(
					ISMInfo[DescIndex],
					const_cast<FHktMassNpcAnimationFragment&>(AnimationData),
					RepresentationLOD.LODSignificance,
					Representation.PrevLODSignificance);
			}

			// Store current state for next frame
			Representation.PrevTransform = TransformFragment.GetTransform();
			Representation.PrevLODSignificance = RepresentationLOD.LODSignificance;
		}
	});
}

void UHktMassNpcUpdateISMBoneAnimationProcessor::UpdateISMBoneAnimation(
	FMassInstancedStaticMeshInfo& ISMInfo,
	FHktMassNpcAnimationFragment& AnimationData,
	const float LODSignificance,
	const float PrevLODSignificance,
	const int32 NumFloatsToPad /*= 0*/)
{
	// Get the AnimToTexture data asset
	const UAnimToTextureDataAsset* AnimToTextureData = AnimationData.AnimToTextureData.Get();
	if (!AnimToTextureData)
	{
		return;
	}

	FAnimToTextureAutoPlayData AutoPlayData;
	if (ensure(UAnimToTextureInstancePlaybackLibrary::GetAutoPlayDataFromDataAsset(AnimToTextureData, AnimationData.AnimationStateIndex, AutoPlayData, AnimationData.GlobalStartTime, AnimationData.PlayRate)) == false)
	{
		return;
	}

	// Add custom data to the ISM
	ISMInfo.AddBatchedCustomData<FAnimToTextureAutoPlayData>(AutoPlayData, LODSignificance, PrevLODSignificance, NumFloatsToPad);
}


