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
struct HKTPROTO_API FHktMassNpcAnimationFragment : public FMassFragment
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

/**
 * Instance playback data structure for bone animation
 * This will be passed to the shader via AddBatchedCustomData
 * Format: TimeOffset, PlayRate, StartFrame, EndFrame
 */
USTRUCT(BlueprintType)
struct FHktMassNpcInstancePlaybackData
{
	GENERATED_BODY()

	// Time offset from global start time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HktMassNpc")
	float TimeOffset = 0.0f;
	
	// Animation playback rate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HktMassNpc")
	float PlayRate = 1.0f;
	
	// Start frame of the animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HktMassNpc")
	float StartFrame = 0.0f;
	
	// End frame of the animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HktMassNpc")
	float EndFrame = 0.0f;
};

