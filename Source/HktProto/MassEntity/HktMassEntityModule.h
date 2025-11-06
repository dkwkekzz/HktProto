// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Mass Entity 서브시스템 초기화 및 설정을 위한 헬퍼 클래스
 */
class FHktMassEntityModule
{
public:
	/** 모듈 초기화 */
	static void Initialize();

	/** 모듈 종료 */
	static void Shutdown();

	/** Mass Entity 시스템이 올바르게 설정되었는지 확인 */
	static bool IsMassEntitySystemValid(UWorld* World);
};

