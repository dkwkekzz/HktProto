// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HktMassNpcSpawnDataAsset.generated.h"

// NPC 스폰 설정 구조체 (Spawner Actor에서 사용)
USTRUCT(BlueprintType)
struct FHktNpcSpawnConfig
{
	GENERATED_BODY()

	// 스폰할 NPC Entity Config
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TObjectPtr<UMassEntityConfigAsset> EntityConfig;

	// 스폰할 NPC 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "1"))
	int32 SpawnCount = 10;

	// 스폰 반경
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0"))
	float SpawnRadius = 1000.0f;

	// 최소 간격
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0"))
	float MinSpacing = 100.0f;
};

/**
 * Mass NPC 스폰 설정을 담는 데이터 애셋
 */
UCLASS(BlueprintType)
class HKTPROTO_API UHktMassNpcSpawnDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TArray<FHktNpcSpawnConfig> SpawnConfigs;
};
