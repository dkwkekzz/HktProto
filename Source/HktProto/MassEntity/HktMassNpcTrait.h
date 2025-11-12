// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassRepresentationTypes.h"
#include "HktMassNpcTrait.generated.h"

class UAnimToTextureDataAsset;

// NPC 엔티티의 기본 구성을 정의하는 Trait
UCLASS(meta = (DisplayName = "Hkt Npc Trait"))
class HKTPROTO_API UHktMassNpcTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	// NPC 타입
	UPROPERTY(EditAnywhere, Category = "NPC")
	uint8 NpcType = 0; // 0: Melee, 1: Ranged, 2: Tank, 3: Support

	UPROPERTY(EditAnywhere, Category = "NPC")
	FStaticMeshInstanceVisualizationDesc NpcMeshDesc;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimToTextureDataAsset> AnimToTextureData;

	// 팀 ID
	UPROPERTY(EditAnywhere, Category = "NPC")
	int32 TeamId = 0;

	// 레벨
	UPROPERTY(EditAnywhere, Category = "NPC")
	int32 Level = 1;

	// 이동 속도
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxSpeed = 400.0f;

	// 체력
	UPROPERTY(EditAnywhere, Category = "Combat")
	float MaxHealth = 100.0f;

	// 공격력
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackPower = 10.0f;

	// 공격 범위
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackRange = 150.0f;

	// 공격 쿨다운
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackCooldown = 1.0f;

	// 순찰 반경
	UPROPERTY(EditAnywhere, Category = "Patrol")
	float PatrolRadius = 500.0f;

	// 순찰 포인트 대기 시간
	UPROPERTY(EditAnywhere, Category = "Patrol")
	float WaitTimeAtPoint = 2.0f;

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};

// 특정 타입의 NPC Trait들
UCLASS(meta = (DisplayName = "Hkt Melee Npc Trait"))
class HKTPROTO_API UHktMassMeleeNpcTrait : public UHktMassNpcTrait
{
	GENERATED_BODY()

public:
	UHktMassMeleeNpcTrait()
	{
		NpcType = 0; // Melee
		MaxSpeed = 350.0f;
		MaxHealth = 150.0f;
		AttackPower = 20.0f;
		AttackRange = 100.0f;
		AttackCooldown = 1.5f;
	}
};

UCLASS(meta = (DisplayName = "Hkt Ranged Npc Trait"))
class HKTPROTO_API UHktMassRangedNpcTrait : public UHktMassNpcTrait
{
	GENERATED_BODY()

public:
	UHktMassRangedNpcTrait()
	{
		NpcType = 1; // Ranged
		MaxSpeed = 300.0f;
		MaxHealth = 80.0f;
		AttackPower = 15.0f;
		AttackRange = 500.0f;
		AttackCooldown = 2.0f;
	}
};

UCLASS(meta = (DisplayName = "Hkt Tank Npc Trait"))
class HKTPROTO_API UHktMassTankNpcTrait : public UHktMassNpcTrait
{
	GENERATED_BODY()

public:
	UHktMassTankNpcTrait()
	{
		NpcType = 2; // Tank
		MaxSpeed = 250.0f;
		MaxHealth = 300.0f;
		AttackPower = 10.0f;
		AttackRange = 100.0f;
		AttackCooldown = 2.5f;
	}
};
