// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassNpcManagerSubsystem.h"
#include "HktMassNpcFragments.h"
#include "HktMassNpcSpawnDataAsset.h"
#include "MassSpawnerSubsystem.h"
#include "MassEntityConfigAsset.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"

void UHktMassNpcManagerSubsystem::Initialize(FSubsystemCollectionBase & Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<UMassSpawnerSubsystem>();
}

void UHktMassNpcManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UHktMassNpcManagerSubsystem::SpawnNpcsFromDataAsset(const UHktMassNpcSpawnDataAsset* SpawnDataAsset, const FVector& CenterLocation)
{
	if (!SpawnDataAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnDataAsset is null!"));
		return;
	}

	for (const FHktNpcSpawnConfig& Config : SpawnDataAsset->SpawnConfigs)
	{
		SpawnNpcsWithConfig(Config, CenterLocation);
	}

	UE_LOG(LogTemp, Log, TEXT("Spawned NPCs from %s"), *SpawnDataAsset->GetName());
}

void UHktMassNpcManagerSubsystem::SpawnNpcsWithConfig(const FHktNpcSpawnConfig& Config, const FVector& CenterLocation)
{
	UMassSpawnerSubsystem* MassSpawnerSubsystem = UWorld::GetSubsystem<UMassSpawnerSubsystem>(GetWorld());
	if (!MassSpawnerSubsystem || !Config.EntityConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot spawn NPCs: Invalid config or subsystem"));
		return;
	}

	TArray<FVector> SpawnPositions = CalculateSpawnPositions(Config, CenterLocation);

	const FMassEntityTemplate& EntityTemplate = Config.EntityConfig->GetConfig().GetOrCreateEntityTemplate(*GetWorld());

	TArray<FMassEntityHandle> Entities;
	MassSpawnerSubsystem->SpawnEntities(EntityTemplate, Config.SpawnCount, Entities);

	FMassEntityManager& EntityManager = MassSpawnerSubsystem->GetEntityManagerChecked();

	for (int32 i = 0; i < Entities.Num(); ++i)
	{
		if (i < SpawnPositions.Num())
		{
			FMassEntityHandle EntityHandle = Entities[i];

			if (FTransformFragment* TransformFragment = EntityManager.GetFragmentDataPtr<FTransformFragment>(EntityHandle))
			{
				FTransform NewTransform(FQuat::Identity, SpawnPositions[i], FVector(1.0f));
				TransformFragment->SetTransform(NewTransform);
			}

			SpawnedEntities.Add(EntityHandle);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("Spawned %d NPCs at location: %s"), Entities.Num(), *CenterLocation.ToString());
}

void UHktMassNpcManagerSubsystem::DespawnAllNpcs()
{
	UMassSpawnerSubsystem* MassSpawnerSubsystem = UWorld::GetSubsystem<UMassSpawnerSubsystem>(GetWorld());
	if (!MassSpawnerSubsystem)
	{
		return;
	}

	MassSpawnerSubsystem->DestroyEntities(SpawnedEntities);
	SpawnedEntities.Empty();

	UE_LOG(LogTemp, Log, TEXT("Despawned all NPCs"));
}

TArray<FVector> UHktMassNpcManagerSubsystem::CalculateSpawnPositions(const FHktNpcSpawnConfig& Config, const FVector& CenterLocation) const
{
	TArray<FVector> Positions;
	Positions.Reserve(Config.SpawnCount);

	const int32 GridSize = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(Config.SpawnCount)));
	const float Spacing = FMath::Max(Config.MinSpacing, 100.0f);

	int32 SpawnedCount = 0;
	for (int32 Y = 0; Y < GridSize && SpawnedCount < Config.SpawnCount; ++Y)
	{
		for (int32 X = 0; X < GridSize && SpawnedCount < Config.SpawnCount; ++X)
		{
			const float OffsetX = (X - GridSize * 0.5f) * Spacing;
			const float OffsetY = (Y - GridSize * 0.5f) * Spacing;

			FVector SpawnLocation = CenterLocation + FVector(OffsetX, OffsetY, 0.0f);

			if (FVector::Distance(CenterLocation, SpawnLocation) <= Config.SpawnRadius)
			{
				const FVector RandomOffset(
					FMath::FRandRange(-Spacing * 0.3f, Spacing * 0.3f),
					FMath::FRandRange(-Spacing * 0.3f, Spacing * 0.3f),
					0.0f);

				SpawnLocation += RandomOffset;
				Positions.Add(SpawnLocation);
				SpawnedCount++;
			}
		}
	}
	return Positions;
}


