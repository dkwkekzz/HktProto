// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassEntityConfigAsset.h"
#include "HktMassNpcSpawner.generated.h"

class UMassEntitySubsystem;
class UInstancedStaticMeshComponent;

// NPC 스폰 설정 구조체
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

// Mass NPC를 생성하는 Spawner 액터
UCLASS()
class HKTPROTO_API AHktMassNpcSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AHktMassNpcSpawner();

	// Melee NPC용 ISM
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInstancedStaticMeshComponent> MeleeNpcMeshISM;

	// Ranged NPC용 ISM
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInstancedStaticMeshComponent> RangedNpcMeshISM;

	// Tank NPC용 ISM
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInstancedStaticMeshComponent> TankNpcMeshISM;

	// 스폰 설정 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TArray<FHktNpcSpawnConfig> SpawnConfigs;

	// 자동 스폰 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	bool bAutoSpawnOnBeginPlay = true;

	// 스폰 딜레이
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float SpawnDelay = 0.0f;

	// 디버그 그리기 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDrawDebugSpawnArea = true;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	// NPC 스폰 함수
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void SpawnNpcs();

	// 특정 설정으로 NPC 스폰
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void SpawnNpcsWithConfig(const FHktNpcSpawnConfig& Config);

	// 모든 스폰된 NPC 제거
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void DespawnAllNpcs();

	// 스폰 위치 계산
	TArray<FVector> CalculateSpawnPositions(const FHktNpcSpawnConfig& Config) const;

private:
	// Mass Entity Subsystem 참조
	UPROPERTY(Transient)
	TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

	// 스폰된 엔티티 핸들 저장
	UPROPERTY(Transient)
	TArray<FMassEntityHandle> SpawnedEntities;

	// 스폰 타이머
	float SpawnTimer = 0.0f;
	bool bHasSpawned = false;
};

