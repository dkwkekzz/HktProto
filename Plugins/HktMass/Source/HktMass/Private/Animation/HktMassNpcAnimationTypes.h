// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "HktMassNpcAnimationTypes.generated.h"

class UAnimToTextureDataAsset;

/**
 * Animation Fragment for NPCs using AnimToTexture
 * Similar to FCrowdAnimationFragment but customized for NPC bone animation
 */
USTRUCT()
struct HKTMASS_API FHktMassNpcAnimationFragment : public FMassFragment
{
	GENERATED_BODY()

	// Reference to the AnimToTexture data asset
	TWeakObjectPtr<UAnimToTextureDataAsset> AnimToTextureData;
	
	// Global start time for animation playback
	UPROPERTY()
	float GlobalStartTime = 0.0f;
	
	// Animation playback rate
	UPROPERTY()
	float PlayRate = 1.0f;
	
	// Current animation state index in the AnimToTextureData
	UPROPERTY()
	int32 AnimationStateIndex = 0;
	
	// Flag to track if animation was swapped this frame
	UPROPERTY()
	bool bSwappedThisFrame = false;
};
