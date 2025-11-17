// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * HktMass ?�러그인 모듈
 * Mass Entity ?�스?�을 ?�용??NPC AI, ?�니메이?? ?�트?�크 복제 ?�스??
 */
class FHktMassModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

