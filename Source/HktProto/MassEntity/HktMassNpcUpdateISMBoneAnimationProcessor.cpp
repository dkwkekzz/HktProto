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
#include "AnimToTextureDataAsset.h"
#include "AnimToTextureInstancePlaybackHelpers.h"

UHktMassNpcUpdateISMBoneAnimationProcessor::UHktMassNpcUpdateISMBoneAnimationProcessor()
{
}

void UHktMassNpcUpdateISMBoneAnimationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::ConfigureQueries(EntityManager);
	
	// Add requirement for our animation fragment
	EntityQuery.AddRequirement<FHktMassNpcAnimationFragment>(EMassFragmentAccess::ReadWrite);
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
		TArrayView<FHktMassNpcAnimationFragment> AnimationDataList = Context.GetMutableFragmentView<FHktMassNpcAnimationFragment>();

		// Iterate through all entities in this chunk
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FTransformFragment& TransformFragment = TransformList[EntityIt];
			const FMassRepresentationLODFragment& RepresentationLOD = RepresentationLODList[EntityIt];
			FMassRepresentationFragment& Representation = RepresentationList[EntityIt];
			FHktMassNpcAnimationFragment& AnimationData = AnimationDataList[EntityIt];

			// Only update if currently represented as static mesh instance
			if (Representation.CurrentRepresentation == EMassRepresentationType::StaticMeshInstance)
			{
				// Update transform
				UpdateISMTransform(Context.GetEntity(EntityIt), ISMInfo[Representation.StaticMeshDescHandle.ToIndex()]
					, TransformFragment.GetTransform(), Representation.PrevTransform, RepresentationLOD.LODSignificance, Representation.PrevLODSignificance);
				
				// Update bone animation
				UpdateISMBoneAnimation(ISMInfo[Representation.StaticMeshDescHandle.ToIndex()], AnimationData, RepresentationLOD.LODSignificance, Representation.PrevLODSignificance);
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

	// Validate animation state index
	if (AnimToTextureData->Animations.IsValidIndex(AnimationData.AnimationStateIndex) == false)
	{
		return;
	}

	if (AnimationData.bSwappedThisFrame)
	{
		FAnimToTextureAutoPlayData AutoPlayData;
		if (UAnimToTextureInstancePlaybackLibrary::GetAutoPlayDataFromDataAsset(AnimToTextureData, 1, AutoPlayData, AnimationData.GlobalStartTime, AnimationData.PlayRate))
		{
			ISMInfo.AddBatchedCustomData<FAnimToTextureAutoPlayData>(AutoPlayData, LODSignificance, PrevLODSignificance, NumFloatsToPad);
		}
	}

	//// Get animation data for current state
	//const FAnimToTextureAnimInfo& AnimInfo = AnimToTextureData->Animations[AnimationData.AnimationStateIndex];
	//
	//// Prepare instance data in the format: TimeOffset, PlayRate, StartFrame, EndFrame
	//FHktMassNpcInstancePlaybackData InstanceData;
	//
	//// Calculate time offset from global start time
	//// Note: World time should be passed in if needed, for now using the GlobalStartTime directly
	//InstanceData.TimeOffset = AnimationData.GlobalStartTime;
	//InstanceData.PlayRate = AnimationData.PlayRate;
	//InstanceData.StartFrame = static_cast<float>(AnimInfo.StartFrame);
	//InstanceData.EndFrame = static_cast<float>(AnimInfo.EndFrame);
	//
	//// Add custom data to the ISM
	//ISMInfo.AddBatchedCustomData<FHktMassNpcInstancePlaybackData>(InstanceData, LODSignificance, PrevLODSignificance, NumFloatsToPad);
}

