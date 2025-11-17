// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "MassProcessor.h"
#include "HktMassNpcUpdateISMBoneAnimationProcessor.generated.h"

struct FMassInstancedStaticMeshInfo;
struct FHktMassNpcAnimationFragment;

/**
 * Processor for updating ISM instances with bone animation data
 * Uses AnimToTexture to apply animations to instanced static meshes
 *
 * This processor handles both Transform and CustomData updates for NPC entities.
 * Only processes entities with FHktMassNpcAnimationFragment, filtering out other Mass entities.
 * This ensures no conflict with generic UMassUpdateISMProcessor.
 */
UCLASS()
class HKTMASS_API UHktMassNpcUpdateISMBoneAnimationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHktMassNpcUpdateISMBoneAnimationProcessor();

	/**
	 * Static method to update ISM with bone animation data
	 * @param ISMInfo The ISM info to update
	 * @param AnimationData The animation fragment containing animation state
	 * @param LODSignificance Current LOD significance
	 * @param PrevLODSignificance Previous LOD significance
	 * @param NumFloatsToPad Number of floats to pad (default 0)
	 */
	static void UpdateISMBoneAnimation(FMassInstancedStaticMeshInfo& ISMInfo,
		FHktMassNpcAnimationFragment& AnimationData,
		const float LODSignificance,
		const float PrevLODSignificance,
		const int32 NumFloatsToPad = 0);

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
