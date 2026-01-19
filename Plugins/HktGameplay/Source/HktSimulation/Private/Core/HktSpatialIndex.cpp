// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktSpatialIndex.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpatialIndex, Log, All);

FHktSpatialIndex::FHktSpatialIndex(float InCellSize, const FBox& InBounds)
	: CellSize(InCellSize)
	, WorldBounds(InBounds)
{
	FVector Size = WorldBounds.GetSize();
	GridDimensions = FIntVector(
		FMath::CeilToInt(Size.X / CellSize),
		FMath::CeilToInt(Size.Y / CellSize),
		FMath::CeilToInt(Size.Z / CellSize)
	);

	UE_LOG(LogSpatialIndex, Log, TEXT("Spatial Index initialized: CellSize=%.1f, GridDim=%s"), 
		CellSize, *GridDimensions.ToString());
}

FIntVector FHktSpatialIndex::WorldToGrid(const FVector& WorldPos) const
{
	FVector RelativePos = WorldPos - WorldBounds.Min;
	return FIntVector(
		FMath::FloorToInt(RelativePos.X / CellSize),
		FMath::FloorToInt(RelativePos.Y / CellSize),
		FMath::FloorToInt(RelativePos.Z / CellSize)
	);
}

int32 FHktSpatialIndex::GridToIndex(const FIntVector& GridCoord) const
{
	// Flatten 3D coordinates to 1D index
	return GridCoord.X + 
		   GridCoord.Y * GridDimensions.X + 
		   GridCoord.Z * GridDimensions.X * GridDimensions.Y;
}

FHktSpatialIndex::FGridCell& FHktSpatialIndex::GetOrCreateCell(const FIntVector& GridCoord)
{
	int32 Index = GridToIndex(GridCoord);
	return Cells.FindOrAdd(Index);
}

const FHktSpatialIndex::FGridCell* FHktSpatialIndex::GetCell(const FIntVector& GridCoord) const
{
	int32 Index = GridToIndex(GridCoord);
	return Cells.Find(Index);
}

void FHktSpatialIndex::Insert(FUnitHandle Handle, const FVector& Location)
{
	FIntVector GridCoord = WorldToGrid(Location);
	FGridCell& Cell = GetOrCreateCell(GridCoord);
	
	Cell.Units.Add(Handle);
	HandleToGrid.Add(HashHandle(Handle), GridCoord);
}

void FHktSpatialIndex::Remove(FUnitHandle Handle)
{
	uint32 Hash = HashHandle(Handle);
	
	if (FIntVector* GridCoord = HandleToGrid.Find(Hash))
	{
		if (FGridCell* Cell = Cells.Find(GridToIndex(*GridCoord)))
		{
			Cell->Units.Remove(Handle);
			
			// Clean up empty cells to save memory
			if (Cell->Units.Num() == 0)
			{
				Cells.Remove(GridToIndex(*GridCoord));
			}
		}
		
		HandleToGrid.Remove(Hash);
	}
}

void FHktSpatialIndex::Update(FUnitHandle Handle, const FVector& OldLocation, const FVector& NewLocation)
{
	FIntVector OldGrid = WorldToGrid(OldLocation);
	FIntVector NewGrid = WorldToGrid(NewLocation);
	
	// If grid cell hasn't changed, no need to update
	if (OldGrid == NewGrid)
	{
		return;
	}
	
	// Remove from old cell and add to new cell
	Remove(Handle);
	Insert(Handle, NewLocation);
}

void FHktSpatialIndex::GetOverlappingCells(const FBox& Box, TArray<FIntVector>& OutCells) const
{
	FIntVector MinGrid = WorldToGrid(Box.Min);
	FIntVector MaxGrid = WorldToGrid(Box.Max);
	
	// Clamp to grid bounds
	MinGrid.X = FMath::Max(0, MinGrid.X);
	MinGrid.Y = FMath::Max(0, MinGrid.Y);
	MinGrid.Z = FMath::Max(0, MinGrid.Z);
	
	MaxGrid.X = FMath::Min(GridDimensions.X - 1, MaxGrid.X);
	MaxGrid.Y = FMath::Min(GridDimensions.Y - 1, MaxGrid.Y);
	MaxGrid.Z = FMath::Min(GridDimensions.Z - 1, MaxGrid.Z);
	
	// Iterate over all cells in the range
	for (int32 X = MinGrid.X; X <= MaxGrid.X; ++X)
	{
		for (int32 Y = MinGrid.Y; Y <= MaxGrid.Y; ++Y)
		{
			for (int32 Z = MinGrid.Z; Z <= MaxGrid.Z; ++Z)
			{
				OutCells.Add(FIntVector(X, Y, Z));
			}
		}
	}
}

void FHktSpatialIndex::QuerySphere(const FVector& Center, float Radius, TArray<FUnitHandle>& OutUnits) const
{
	// Create bounding box for the sphere
	FBox QueryBox(Center - FVector(Radius), Center + FVector(Radius));
	
	// Get all cells that overlap with the box
	TArray<FIntVector> OverlappingCells;
	GetOverlappingCells(QueryBox, OverlappingCells);
	
	float RadiusSq = Radius * Radius;
	
	// Check all units in overlapping cells
	TSet<uint32> AddedHandles; // Avoid duplicates if a unit is in multiple cells
	
	for (const FIntVector& GridCoord : OverlappingCells)
	{
		if (const FGridCell* Cell = GetCell(GridCoord))
		{
			for (const FUnitHandle& Handle : Cell->Units)
			{
				uint32 Hash = HashHandle(Handle);
				if (!AddedHandles.Contains(Hash))
				{
					// Note: We don't have access to actual locations here
					// In a real implementation, you'd pass the EntityDatabase or check distance
					// For now, we add all units in overlapping cells
					OutUnits.Add(Handle);
					AddedHandles.Add(Hash);
				}
			}
		}
	}
}

void FHktSpatialIndex::QueryBox(const FBox& Box, TArray<FUnitHandle>& OutUnits) const
{
	// Get all cells that overlap with the box
	TArray<FIntVector> OverlappingCells;
	GetOverlappingCells(Box, OverlappingCells);
	
	// Collect all units in overlapping cells
	TSet<uint32> AddedHandles; // Avoid duplicates
	
	for (const FIntVector& GridCoord : OverlappingCells)
	{
		if (const FGridCell* Cell = GetCell(GridCoord))
		{
			for (const FUnitHandle& Handle : Cell->Units)
			{
				uint32 Hash = HashHandle(Handle);
				if (!AddedHandles.Contains(Hash))
				{
					OutUnits.Add(Handle);
					AddedHandles.Add(Hash);
				}
			}
		}
	}
}

void FHktSpatialIndex::Clear()
{
	Cells.Empty();
	HandleToGrid.Empty();
}

void FHktSpatialIndex::GetStats(int32& OutTotalUnits, int32& OutOccupiedCells, int32& OutMaxUnitsPerCell) const
{
	OutTotalUnits = HandleToGrid.Num();
	OutOccupiedCells = Cells.Num();
	OutMaxUnitsPerCell = 0;
	
	for (const auto& Pair : Cells)
	{
		OutMaxUnitsPerCell = FMath::Max(OutMaxUnitsPerCell, Pair.Value.Units.Num());
	}
}
