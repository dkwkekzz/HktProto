// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Mass Entity ?œë¸Œ?œìŠ¤??ì´ˆê¸°??ë°??¤ì •???„í•œ ?¬í¼ ?´ë˜??
 */
class FHktMassEntityModule
{
public:
	/** ëª¨ë“ˆ ì´ˆê¸°??*/
	static void Initialize();

	/** ëª¨ë“ˆ ì¢…ë£Œ */
	static void Shutdown();

	/** Mass Entity ?œìŠ¤?œì´ ?¬ë°”ë¥´ê²Œ ?¤ì •?˜ì—ˆ?”ì? ?•ì¸ */
	static bool IsMassEntitySystemValid(UWorld* World);
};

