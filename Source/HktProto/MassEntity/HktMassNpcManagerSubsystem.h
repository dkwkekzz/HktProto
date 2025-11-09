// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassEntityHandle.h"
#include "HktMassNpcManagerSubsystem.generated.h"

class UInstancedStaticMeshComponent;
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

	TObjectPtr<UInstancedStaticMeshComponent> GetMeleeNpcMeshISM() const { return MeleeNpcMeshISM; }
    TObjectPtr<UInstancedStaticMeshComponent> GetRangedNpcMeshISM() const { return RangedNpcMeshISM; }
    TObjectPtr<UInstancedStaticMeshComponent> GetTankNpcMeshISM() const { return TankNpcMeshISM; }

private:
	void SpawnNpcsWithConfig(const FHktNpcSpawnConfig& Config, const FVector& CenterLocation);
	TArray<FVector> CalculateSpawnPositions(const FHktNpcSpawnConfig& Config, const FVector& CenterLocation) const;

	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> MeleeNpcMeshISM;
    
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> RangedNpcMeshISM;

	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> TankNpcMeshISM;

	UPROPERTY(Transient)
	TObjectPtr<UMassSpawnerSubsystem> MassSpawnerSubsystem;

	UPROPERTY(Transient)
	TArray<FMassEntityHandle> SpawnedEntities;

	// ISM 컴포넌트를 소유할 액터
	UPROPERTY()
	TObjectPtr<AActor> IsmOwnerActor;
};

