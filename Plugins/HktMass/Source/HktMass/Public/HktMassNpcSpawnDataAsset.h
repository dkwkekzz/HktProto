// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MassEntityConfigAsset.h"
#include "HktMassNpcSpawnDataAsset.generated.h"

// NPC ?§Ìè∞ ?§Ï†ï Íµ¨Ï°∞Ï≤?(Spawner Actor?êÏÑú ?¨Ïö©)
USTRUCT(BlueprintType)
struct FHktNpcSpawnConfig
{
	GENERATED_BODY()

	// ?§Ìè∞??NPC Entity Config
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TObjectPtr<UMassEntityConfigAsset> EntityConfig;

	// ?§Ìè∞??NPC ??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "1"))
	int32 SpawnCount = 10;

	// ?§Ìè∞ Î∞òÍ≤Ω
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0"))
	float SpawnRadius = 1000.0f;

	// ÏµúÏÜå Í∞ÑÍ≤©
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0"))
	float MinSpacing = 100.0f;
};

/**
 * Mass NPC ?§Ìè∞ ?§Ï†ï???¥Îäî ?∞Ïù¥???†ÏÖã
 */
UCLASS(BlueprintType)
class HKTMASS_API UHktMassNpcSpawnDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TArray<FHktNpcSpawnConfig> SpawnConfigs;
};
