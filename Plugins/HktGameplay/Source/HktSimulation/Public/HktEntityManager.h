// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HktEntityDatabase.h"

/**
 * [System]
 * EntityDB와 PlayerDB를 소유하고 관리하는 매니저 클래스.
 * VM은 이 클래스를 통해 데이터에 접근합니다.
 */
class HKTSIMULATION_API FHktEntityManager
{
public:
	FHktEntityDatabase Entities;
	FHktPlayerDatabase Players;

	// Management
	TArray<int32> FreeEntityIndices;

	// --- Player Management ---
	
	FPlayerHandle AllocPlayer();
	FHktAttributeSet* GetPlayerAttrs(FPlayerHandle Handle);

	// --- Unit Management ---

	FUnitHandle AllocUnit(FPlayerHandle Owner, FVector SpawnLoc, FRotator SpawnRot);
	void FreeUnit(FUnitHandle Handle);
	bool IsUnitValid(FUnitHandle Handle) const;

	// --- Unit Accessors ---

	FHktAttributeSet* GetUnitAttrs(FUnitHandle Handle);
	FVector GetUnitLocation(FUnitHandle Handle) const;
	FRotator GetUnitRotation(FUnitHandle Handle) const;

	// 유닛 소유주 찾기
	FPlayerHandle GetUnitOwner(FUnitHandle Handle) const;

	void SetVisualActor(FUnitHandle Handle, AActor* Actor);
	AActor* GetVisualActor(FUnitHandle Handle);
	
	// 유닛 위치 동기화 (Visual -> Logic)
	void SyncLocationFromVisual(FUnitHandle Handle);
};
