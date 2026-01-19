// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HktEntityDatabase.h"

/**
 * Grid-based spatial partitioning for efficient range queries
 * Optimizes O(N) linear searches to O(K) where K << N
 */
class HKTSIMULATION_API FHktSpatialIndex
{
public:
	/**
	 * Constructor
	 * @param InCellSize - Size of each grid cell (default: 1000 units)
	 * @param InBounds - World bounds for the grid (default: -50000 to 50000)
	 */
	FHktSpatialIndex(float InCellSize = 1000.0f, const FBox& InBounds = FBox(FVector(-50000), FVector(50000)));

	/**
	 * Insert a unit into the spatial index
	 * @param Handle - The unit handle
	 * @param Location - The unit's location
	 */
	void Insert(FUnitHandle Handle, const FVector& Location);

	/**
	 * Remove a unit from the spatial index
	 * @param Handle - The unit handle
	 */
	void Remove(FUnitHandle Handle);

	/**
	 * Update a unit's location in the spatial index
	 * @param Handle - The unit handle
	 * @param OldLocation - The unit's previous location
	 * @param NewLocation - The unit's new location
	 */
	void Update(FUnitHandle Handle, const FVector& OldLocation, const FVector& NewLocation);

	/**
	 * Query units within a sphere
	 * @param Center - Center of the sphere
	 * @param Radius - Radius of the sphere
	 * @param OutUnits - Output array of unit handles
	 */
	void QuerySphere(const FVector& Center, float Radius, TArray<FUnitHandle>& OutUnits) const;

	/**
	 * Query units within a box
	 * @param Box - The bounding box
	 * @param OutUnits - Output array of unit handles
	 */
	void QueryBox(const FBox& Box, TArray<FUnitHandle>& OutUnits) const;

	/**
	 * Clear all units from the index
	 */
	void Clear();

	/**
	 * Get statistics about the spatial index
	 */
	void GetStats(int32& OutTotalUnits, int32& OutOccupiedCells, int32& OutMaxUnitsPerCell) const;

private:
	struct FGridCell
	{
		TArray<FUnitHandle> Units;
	};

	/**
	 * Convert world position to grid coordinates
	 */
	FIntVector WorldToGrid(const FVector& WorldPos) const;

	/**
	 * Convert grid coordinates to cell index
	 */
	int32 GridToIndex(const FIntVector& GridCoord) const;

	/**
	 * Get the cell at grid coordinates (creates if doesn't exist)
	 */
	FGridCell& GetOrCreateCell(const FIntVector& GridCoord);

	/**
	 * Get the cell at grid coordinates (read-only)
	 */
	const FGridCell* GetCell(const FIntVector& GridCoord) const;

	/**
	 * Get grid coordinates that overlap with a box
	 */
	void GetOverlappingCells(const FBox& Box, TArray<FIntVector>& OutCells) const;

	// Grid configuration
	float CellSize;
	FBox WorldBounds;
	FIntVector GridDimensions;

	// Cell storage: Index -> Cell
	TMap<int32, FGridCell> Cells;

	// Reverse lookup: Handle -> Grid Coordinate
	TMap<uint32, FIntVector> HandleToGrid;

	/**
	 * Hash a unit handle for fast lookup
	 */
	FORCEINLINE uint32 HashHandle(FUnitHandle Handle) const
	{
		return HashCombine(Handle.Index, Handle.Generation);
	}
};
