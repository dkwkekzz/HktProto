// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "MassUpdateISMProcessor.h"
#include "HktMassNpcUpdateISMBoneAnimationProcessor.generated.h"

struct FMassInstancedStaticMeshInfo;
struct FHktMassNpcAnimationFragment;

/**
 * Processor for updating ISM instances with bone animation data
 * Uses AnimToTexture to apply animations to instanced static meshes
 * Based on UMassCrowdUpdateISMVertexAnimationProcessor but customized for NPC bone animation
 */
UCLASS()
class HKTPROTO_API UHktMassNpcUpdateISMBoneAnimationProcessor : public UMassUpdateISMProcessor
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
	/** Configure the owned FMassEntityQuery instances to express processor's requirements */
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;

	/**
	 * Execution method for this processor
	 * @param EntityManager The entity manager
	 * @param Context The execution context to be passed when executing the lambdas
	 */
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};
