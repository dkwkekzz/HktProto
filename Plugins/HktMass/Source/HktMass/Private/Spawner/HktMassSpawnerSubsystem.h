// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassEntityHandle.h"
#include "HktMassSpawnerSubsystem.generated.h"

class UMassSpawnerSubsystem;
class UMassEntityConfigAsset;

// Spawn Config
USTRUCT(BlueprintType)
struct FHktSpawnConfig
{
	GENERATED_BODY()

	// Spawn Entity Config
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TObjectPtr<UMassEntityConfigAsset> EntityConfig;

	// Spawn Count
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "1"))
	int32 SpawnCount = 10;

	// Spawn Radius
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0"))
	float SpawnRadius = 1000.0f;

	// Min Spacing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0"))
	float MinSpacing = 100.0f;
};

UCLASS()
class HKTMASS_API UHktMassSpawnerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "HktMassSpawner")
	void SpawnEntitiesFromConfig(const FHktSpawnConfig& SpawnConfig, const FVector& CenterLocation);

	UFUNCTION(BlueprintCallable, Category = "HktMassSpawner")
	void DespawnAllEntities();

private:
	TArray<FVector> CalculateSpawnPositions(const FHktSpawnConfig& Config, const FVector& CenterLocation) const;

	UPROPERTY(Transient)
	TArray<FMassEntityHandle> SpawnedEntities;
};
