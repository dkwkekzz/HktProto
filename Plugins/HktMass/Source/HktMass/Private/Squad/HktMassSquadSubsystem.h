// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "HktMassSquadSubsystem.generated.h"

/**
 * 분대(Squad) 시스템을 관리하는 서브시스템.
 * 분대 관련 공통 로직이나 유틸리티 기능을 제공합니다.
 * (Entity Handle 기반으로 변경되어 ID 맵 관리 기능은 제거됨)
 */
UCLASS()
class HKTMASS_API UHktMassSquadSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};
