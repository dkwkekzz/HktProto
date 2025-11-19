// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassNpcAnimationTrait.h"
#include "HktMassNpcAnimationTypes.h"
#include "HktMassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "AnimToTextureDataAsset.h"

void UHktMassNpcAnimationTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FHktNpcStateFragment>();

	// Animation Fragment 추�?
	if (AnimToTextureData)
	{
		FHktMassNpcAnimationFragment& AnimationFragment = BuildContext.AddFragment_GetRef<FHktMassNpcAnimationFragment>();
		AnimationFragment.AnimToTextureData = AnimToTextureData;
	}
}

