// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassNpcSpawner.h"
#include "HktMassNpcFragments.h"
#include "HktMassNpcVisualizationProcessor.h"
#include "MassEntitySubsystem.h"
#include "MassEntityConfigAsset.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "DrawDebugHelpers.h"

AHktMassNpcSpawner::AHktMassNpcSpawner()
{
	PrimaryActorTick.bCanEverTick = true;

	// Root 컴포넌트 설정
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// ISM 컴포넌트 생성
	MeleeNpcMeshISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("MeleeNpcMeshISM"));
	MeleeNpcMeshISM->SetupAttachment(RootComponent);
	MeleeNpcMeshISM->SetCastShadow(true);
	MeleeNpcMeshISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RangedNpcMeshISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RangedNpcMeshISM"));
	RangedNpcMeshISM->SetupAttachment(RootComponent);
	RangedNpcMeshISM->SetCastShadow(true);
	RangedNpcMeshISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TankNpcMeshISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TankNpcMeshISM"));
	TankNpcMeshISM->SetupAttachment(RootComponent);
	TankNpcMeshISM->SetCastShadow(true);
	TankNpcMeshISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AHktMassNpcSpawner::BeginPlay()
{
	Super::BeginPlay();

	// Mass Entity Subsystem 가져오기
	MassEntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(GetWorld());

	if (bAutoSpawnOnBeginPlay && SpawnDelay <= 0.0f)
	{
		SpawnNpcs();
		bHasSpawned = true;
	}
}

void AHktMassNpcSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 지연 스폰 처리
	if (bAutoSpawnOnBeginPlay && !bHasSpawned && SpawnDelay > 0.0f)
	{
		SpawnTimer += DeltaTime;
		if (SpawnTimer >= SpawnDelay)
		{
			SpawnNpcs();
			bHasSpawned = true;
		}
	}

	// 디버그 그리기
#if WITH_EDITOR
	if (bDrawDebugSpawnArea)
	{
		for (const FHktNpcSpawnConfig& Config : SpawnConfigs)
		{
			DrawDebugSphere(GetWorld(), GetActorLocation(), Config.SpawnRadius, 32, FColor::Green, false, -1.0f, 0, 2.0f);
		}
	}
#endif
}

#if WITH_EDITOR
void AHktMassNpcSpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 에디터에서 프로퍼티 변경 시 처리
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AHktMassNpcSpawner, SpawnConfigs))
	{
		// 스폰 설정 변경 시 처리
	}
}
#endif

void AHktMassNpcSpawner::SpawnNpcs()
{
	if (!MassEntitySubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("MassEntitySubsystem is null!"));
		return;
	}

	// 모든 스폰 설정에 대해 NPC 생성
	for (const FHktNpcSpawnConfig& Config : SpawnConfigs)
	{
		SpawnNpcsWithConfig(Config);
	}

	UE_LOG(LogTemp, Log, TEXT("Spawned NPCs from %d configs"), SpawnConfigs.Num());
}

void AHktMassNpcSpawner::SpawnNpcsWithConfig(const FHktNpcSpawnConfig& Config)
{
	if (!MassEntitySubsystem || !Config.EntityConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot spawn NPCs: Invalid config or subsystem"));
		return;
	}

	// 스폰 위치 계산
	TArray<FVector> SpawnPositions = CalculateSpawnPositions(Config);

	// Entity 생성
	const FMassEntityTemplate& EntityTemplate = Config.EntityConfig->GetConfig().GetOrCreateEntityTemplate(*GetWorld());
	
	// 엔티티 배치로 생성
	FMassEntityManager& EntityManager = MassEntitySubsystem->GetMutableEntityManager();
	TArray<FMassEntityHandle> Entities;
	EntityManager.BatchCreateEntities(EntityTemplate.GetArchetype(), Config.SpawnCount, Entities);

	// 각 엔티티의 초기 위치 설정
	for (int32 i = 0; i < Entities.Num(); ++i)
	{
		if (i < SpawnPositions.Num())
		{
			FMassEntityHandle EntityHandle = Entities[i];
			
			// Transform Fragment 설정
			if (FTransformFragment* TransformFragment = EntityManager.GetFragmentDataPtr<FTransformFragment>(EntityHandle))
			{
				FTransform NewTransform;
				NewTransform.SetLocation(SpawnPositions[i]);
				NewTransform.SetRotation(FQuat::Identity);
				NewTransform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));
				TransformFragment->SetTransform(NewTransform);
			}

			// Visualization Fragment 설정
			if (FHktNpcVisualizationFragment* VisualizationFragment = EntityManager.GetFragmentDataPtr<FHktNpcVisualizationFragment>(EntityHandle))
			{
				// ISM 인스턴스 추가
				const FHktNpcTypeFragment* TypeFragment = EntityManager.GetFragmentDataPtr<FHktNpcTypeFragment>(EntityHandle);
				if (TypeFragment)
				{
					UInstancedStaticMeshComponent* TargetISM = nullptr;
					
					switch (TypeFragment->NpcType)
					{
						case 0: // Melee
							TargetISM = MeleeNpcMeshISM;
							break;
						case 1: // Ranged
							TargetISM = RangedNpcMeshISM;
							break;
						case 2: // Tank
							TargetISM = TankNpcMeshISM;
							break;
						default:
							TargetISM = MeleeNpcMeshISM;
							break;
					}

					if (TargetISM)
					{
						FTransform InstanceTransform;
						InstanceTransform.SetLocation(SpawnPositions[i]);
						InstanceTransform.SetRotation(FQuat::Identity);
						InstanceTransform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));
						
						const int32 InstanceIndex = TargetISM->AddInstance(InstanceTransform, true);
						VisualizationFragment->InstanceIndex = InstanceIndex;
					}
				}
			}

			SpawnedEntities.Add(EntityHandle);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Spawned %d NPCs at location: %s"), Entities.Num(), *GetActorLocation().ToString());
}

void AHktMassNpcSpawner::DespawnAllNpcs()
{
	if (!MassEntitySubsystem)
	{
		return;
	}

	FMassEntityManager& EntityManager = MassEntitySubsystem->GetMutableEntityManager();

	// 모든 스폰된 엔티티 제거
	for (FMassEntityHandle EntityHandle : SpawnedEntities)
	{
		if (EntityManager.IsEntityValid(EntityHandle))
		{
			EntityManager.DestroyEntity(EntityHandle);
		}
	}

	SpawnedEntities.Empty();

	// ISM 인스턴스 클리어
	if (MeleeNpcMeshISM)
	{
		MeleeNpcMeshISM->ClearInstances();
	}
	if (RangedNpcMeshISM)
	{
		RangedNpcMeshISM->ClearInstances();
	}
	if (TankNpcMeshISM)
	{
		TankNpcMeshISM->ClearInstances();
	}

	bHasSpawned = false;
	SpawnTimer = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("Despawned all NPCs"));
}

TArray<FVector> AHktMassNpcSpawner::CalculateSpawnPositions(const FHktNpcSpawnConfig& Config) const
{
	TArray<FVector> Positions;
	Positions.Reserve(Config.SpawnCount);

	const FVector CenterLocation = GetActorLocation();
	
	// 간단한 그리드 기반 배치
	const int32 GridSize = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(Config.SpawnCount)));
	const float Spacing = FMath::Max(Config.MinSpacing, 100.0f);

	int32 SpawnedCount = 0;
	for (int32 Y = 0; Y < GridSize && SpawnedCount < Config.SpawnCount; ++Y)
	{
		for (int32 X = 0; X < GridSize && SpawnedCount < Config.SpawnCount; ++X)
		{
			// 그리드 중심을 기준으로 배치
			const float OffsetX = (X - GridSize * 0.5f) * Spacing;
			const float OffsetY = (Y - GridSize * 0.5f) * Spacing;
			
			FVector SpawnLocation = CenterLocation + FVector(OffsetX, OffsetY, 0.0f);
			
			// 스폰 반경 내에 있는지 확인
			const float Distance = FVector::Distance(CenterLocation, SpawnLocation);
			if (Distance <= Config.SpawnRadius)
			{
				// 약간의 랜덤 오프셋 추가
				const FVector RandomOffset = FVector(
					FMath::FRandRange(-Spacing * 0.3f, Spacing * 0.3f),
					FMath::FRandRange(-Spacing * 0.3f, Spacing * 0.3f),
					0.0f
				);
				
				SpawnLocation += RandomOffset;
				Positions.Add(SpawnLocation);
				SpawnedCount++;
			}
		}
	}

	return Positions;
}

