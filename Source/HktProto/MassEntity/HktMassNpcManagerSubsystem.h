// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassEntityHandle.h"
#include "HktMassNpcManagerSubsystem.generated.h"

class UMassSpawnerSubsystem;
class UMassEntityConfigAsset;
class UHktMassNpcSpawnDataAsset;
struct FHktNpcSpawnConfig;

UCLASS()
class HKTPROTO_API UHktMassNpcManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "HktMassNPC")
	void SpawnNpcsFromDataAsset(const UHktMassNpcSpawnDataAsset* SpawnDataAsset, const FVector& CenterLocation);

	void DespawnAllNpcs();

private:
	void SpawnNpcsWithConfig(const FHktNpcSpawnConfig& Config, const FVector& CenterLocation);
	TArray<FVector> CalculateSpawnPositions(const FHktNpcSpawnConfig& Config, const FVector& CenterLocation) const;

	UPROPERTY(Transient)
	TArray<FMassEntityHandle> SpawnedEntities;
};
