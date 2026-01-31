// Copyright Hkt Studios, Inc. All Rights Reserved.
// 이 파일을 HktRuntime/Public/ 에 추가하세요

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HktRuntimeTypes.h"
#include "HktRuntimeInterfaces.h"

// ============================================================================
// 헬퍼 함수
// ============================================================================

namespace HktRuntimeHelper
{
	HKTRUNTIME_API TScriptInterface<IHktModelProvider> GetModelProvider(UWorld* InWorld);
}
