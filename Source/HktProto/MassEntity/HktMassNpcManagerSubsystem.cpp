// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassNpcManagerSubsystem.h"
#include "HktMassNpcFragments.h"
#include "HktMassNpcSpawnDataAsset.h"
#include "HktMassNpcVisualizationProcessor.h"
#include "MassSpawnerSubsystem.h"
#include "MassEntityConfigAsset.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"

void UHktMassNpcManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	MassSpawnerSubsystem = UWorld::GetSubsystem<UMassSpawnerSubsystem>(GetWorld());

	// ISM 컴포넌트를 소유할 임시 액터를 생성합니다.
	IsmOwnerActor = GetWorld()->SpawnActor<AActor>();
	
	// ISM 컴포넌트 생성 및 설정
	MeleeNpcMeshISM = NewObject<UInstancedStaticMeshComponent>(IsmOwnerActor, TEXT("MeleeNpcMeshISM"));
	MeleeNpcMeshISM->RegisterComponent();
	MeleeNpcMeshISM->SetCastShadow(true);
	MeleeNpcMeshISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RangedNpcMeshISM = NewObject<UInstancedStaticMeshComponent>(IsmOwnerActor, TEXT("RangedNpcMeshISM"));
	RangedNpcMeshISM->RegisterComponent();
	RangedNpcMeshISM->SetCastShadow(true);
	RangedNpcMeshISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TankNpcMeshISM = NewObject<UInstancedStaticMeshComponent>(IsmOwnerActor, TEXT("TankNpcMeshISM"));
	TankNpcMeshISM->RegisterComponent();
	TankNpcMeshISM->SetCastShadow(true);
	TankNpcMeshISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UHktMassNpcManagerSubsystem::Deinitialize()
{
	if (IsmOwnerActor)
	{
		IsmOwnerActor->Destroy();
		IsmOwnerActor = nullptr;
	}

	Super::Deinitialize();
}

void UHktMassNpcManagerSubsystem::SpawnNpcsFromDataAsset(const UHktMassNpcSpawnDataAsset* SpawnDataAsset, const FVector& CenterLocation)
{
	if (!MassSpawnerSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("MassSpawnerSubsystem is null!"));
		return;
	}

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

			if (FHktNpcVisualizationFragment* VisualizationFragment = EntityManager.GetFragmentDataPtr<FHktNpcVisualizationFragment>(EntityHandle))
			{
				const FHktNpcTypeFragment* TypeFragment = EntityManager.GetFragmentDataPtr<FHktNpcTypeFragment>(EntityHandle);
				if (TypeFragment)
				{
					UInstancedStaticMeshComponent* TargetISM = nullptr;
					switch (TypeFragment->NpcType)
					{
						case 0: TargetISM = MeleeNpcMeshISM; break;
						case 1: TargetISM = RangedNpcMeshISM; break;
						case 2: TargetISM = TankNpcMeshISM; break;
						default: TargetISM = MeleeNpcMeshISM; break;
					}

					if (TargetISM)
					{
						FTransform InstanceTransform(FQuat::Identity, SpawnPositions[i], FVector(1.0f));
						const int32 InstanceIndex = TargetISM->AddInstance(InstanceTransform, true);
						VisualizationFragment->InstanceIndex = InstanceIndex;
					}
				}
			}
			SpawnedEntities.Add(EntityHandle);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("Spawned %d NPCs at location: %s"), Entities.Num(), *CenterLocation.ToString());
}

void UHktMassNpcManagerSubsystem::DespawnAllNpcs()
{
	if (!MassSpawnerSubsystem)
	{
		return;
	}

	MassSpawnerSubsystem->DestroyEntities(SpawnedEntities);
	SpawnedEntities.Empty();

	if (MeleeNpcMeshISM) MeleeNpcMeshISM->ClearInstances();
	if (RangedNpcMeshISM) RangedNpcMeshISM->ClearInstances();
	if (TankNpcMeshISM) TankNpcMeshISM->ClearInstances();

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

