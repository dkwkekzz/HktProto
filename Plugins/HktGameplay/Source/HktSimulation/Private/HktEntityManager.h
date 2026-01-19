// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HktEntityDatabase.h"

class FHktSpatialIndex;

/**
 * [System]
 * EntityDB와 PlayerDB를 소유하고 관리하는 매니저 클래스.
 * VM은 이 클래스를 통해 데이터에 접근합니다.
 */
class HKTSIMULATION_API FHktEntityManager
{
public:
	FHktEntityManager();
	~FHktEntityManager();

	FHktEntityDatabase Entities;
	FHktPlayerDatabase Players;

	// Management
	TArray<int32> FreeEntityIndices;

	// --- Player Management ---
	
	FPlayerHandle AllocPlayer();
	void FreePlayer(FPlayerHandle Handle);
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

	// --- Spatial Queries ---

	/** 
	 * Query units within a sphere (optimized with spatial index)
	 * @param Center - Center of the sphere
	 * @param Radius - Radius of the sphere
	 * @param OutUnits - Output array of unit handles
	 * @param bFilterActive - If true, only return active units (default: true)
	 */
	void QueryUnitsInSphere(const FVector& Center, float Radius, TArray<FUnitHandle>& OutUnits, bool bFilterActive = true) const;

	/**
	 * Query units within a box (optimized with spatial index)
	 */
	void QueryUnitsInBox(const FBox& Box, TArray<FUnitHandle>& OutUnits, bool bFilterActive = true) const;

	/**
	 * Enable or disable spatial indexing (default: enabled)
	 */
	void SetSpatialIndexEnabled(bool bEnabled);

	/**
	 * Check if spatial indexing is enabled
	 */
	bool IsSpatialIndexEnabled() const { return bUseSpatialIndex; }

private:
	// Spatial index for optimized range queries
	TUniquePtr<FHktSpatialIndex> SpatialIndex;
	bool bUseSpatialIndex = true;
};
