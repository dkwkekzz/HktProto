// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassCommonTypes.h"

namespace HktMass
{
	namespace ExecuteGroupNames
	{
		const FName AI = FName(TEXT("AI"));
		const FName Animation = FName(TEXT("Animation"));
		const FName Collision = FName(TEXT("Collision"));
		const FName Movement = FName(TEXT("Movement"));
		const FName Physics_ApplyVelocity = FName(TEXT("Physics_ApplyVelocity"));
		const FName Physics_ApplyTransform = FName(TEXT("Physics_ApplyTransform"));
		const FName Physics_DebugVisualization = FName(TEXT("Physics_DebugVisualization"));
		const FName Representation = FName(TEXT("Representation"));
		const FName Squad = FName(TEXT("Squad"));
	}
}