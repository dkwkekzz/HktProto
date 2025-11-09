// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassRepresentationProcessor.h"
#include "HktMassNpcVisualizationProcessor.generated.h"

class UHktMassNpcManagerSubsystem;

// NPC의 시각적 표현을 담는 Fragment
USTRUCT()
struct FHktNpcVisualizationFragment : public FMassFragment
{
	GENERATED_BODY()

	// 메시 컴포넌트 참조 (ISM 인덱스)
	UPROPERTY()
	int32 InstanceIndex = -1;

	// 메시 타입
	UPROPERTY()
	uint8 MeshType = 0;

	// 스케일
	UPROPERTY()
	FVector Scale = FVector(1.0f, 1.0f, 1.0f);
};

// NPC 시각화를 처리하는 Processor
UCLASS()
class HKTPROTO_API UHktMassNpcVisualizationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHktMassNpcVisualizationProcessor();

protected:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
	
	UPROPERTY()
	TObjectPtr<UHktMassNpcManagerSubsystem> NpcManagerSubsystem;

	// Instanced Static Mesh Component 참조들
	UPROPERTY(Transient)
	TObjectPtr<UInstancedStaticMeshComponent> MeleeNpcMeshISM;

	UPROPERTY(Transient)
	TObjectPtr<UInstancedStaticMeshComponent> RangedNpcMeshISM;
};

