// Copyright Hkt Studios, Inc. All Rights Reserved.
// 이 파일을 HktRuntime/Private/ 에 추가하세요

#include "HktRuntimeHelper.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

TScriptInterface<IHktModelProvider> HktRuntimeHelper::GetModelProvider(UWorld* InWorld)
{
	if (!InWorld)
	{
		return nullptr;
	}

	// 로컬 플레이어 컨트롤러에서 찾기
	return InWorld->GetFirstPlayerController();
}
