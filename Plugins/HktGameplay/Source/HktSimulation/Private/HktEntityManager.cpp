// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktEntityManager.h"
#include "Core/HktSpatialIndex.h"
#include "Core/HktSimulationStats.h"
#include "GameFramework/Actor.h"

FHktEntityManager::FHktEntityManager()
{
	// Initialize spatial index with default parameters
	SpatialIndex = MakeUnique<FHktSpatialIndex>();
}

FHktEntityManager::~FHktEntityManager()
{
}

FPlayerHandle FHktEntityManager::AllocPlayer()
{
	int32 Idx = Players.AddPlayer();
	return FPlayerHandle(Idx);
}

void FHktEntityManager::FreePlayer(FPlayerHandle Handle)
{
	if (Handle.IsValid())
	{
		Players.RemovePlayer(Handle.Index);
	}
}

FHktAttributeSet* FHktEntityManager::GetPlayerAttrs(FPlayerHandle Handle)
{
	if (Handle.IsValid() && Players.IsActive.IsValidIndex(Handle.Index) && Players.IsActive[Handle.Index])
	{
		// Mark as dirty when attributes are accessed for modification
		// Caller is expected to modify attributes
		Players.MarkDirty(Handle.Index, EPlayerDirtyFlag::Attributes);
		return &Players.Attributes[Handle.Index];
	}
	return nullptr;
}

FUnitHandle FHktEntityManager::AllocUnit(FPlayerHandle Owner, FVector SpawnLoc, FRotator SpawnRot)
{
	int32 Idx;
	if (FreeEntityIndices.Num() > 0)
	{
		Idx = FreeEntityIndices.Pop();
		Entities.Generations[Idx]++;
	}
	else
	{
		Idx = Entities.Attributes.Add(FHktAttributeSet());
		Entities.Locations.Add(SpawnLoc);
		Entities.Rotations.Add(SpawnRot);
		Entities.OwnerPlayerIndices.Add(INDEX_NONE);
		Entities.Generations.Add(1);
		Entities.IsActive.Add(true);
		Entities.ExternalIds.Add(INDEX_NONE); // [New] External ID storage
		Entities.VisualActors.Add(nullptr);
	}

	// Initialize
	Entities.IsActive[Idx] = true;
	Entities.Locations[Idx] = SpawnLoc;
	Entities.Rotations[Idx] = SpawnRot;
	Entities.OwnerPlayerIndices[Idx] = Owner.IsValid() ? Owner.Index : INDEX_NONE;
	Entities.Attributes[Idx] = FHktAttributeSet(); // Reset
	Entities.ExternalIds[Idx] = INDEX_NONE; // Will be set by caller if needed
	Entities.VisualActors[Idx] = nullptr;

	FUnitHandle NewHandle(Idx, Entities.Generations[Idx]);

	// Add to spatial index
	if (bUseSpatialIndex && SpatialIndex.IsValid())
	{
		SpatialIndex->Insert(NewHandle, SpawnLoc);
	}

	return NewHandle;
}

void FHktEntityManager::FreeUnit(FUnitHandle Handle)
{
	if (!IsUnitValid(Handle)) return;
	
	// Remove from spatial index
	if (bUseSpatialIndex && SpatialIndex.IsValid())
	{
		SpatialIndex->Remove(Handle);
	}

	Entities.IsActive[Handle.Index] = false;
	
	// Destroy Visual
	if (AActor* Actor = Entities.VisualActors[Handle.Index].Get())
	{
		Actor->Destroy();
	}
	Entities.VisualActors[Handle.Index] = nullptr;
	
	FreeEntityIndices.Add(Handle.Index);
}

bool FHktEntityManager::IsUnitValid(FUnitHandle Handle) const
{
	return Handle.Index != INDEX_NONE &&
		   Entities.IsActive.IsValidIndex(Handle.Index) &&
		   Entities.IsActive[Handle.Index] &&
		   Entities.Generations[Handle.Index] == Handle.Generation;
}

FHktAttributeSet* FHktEntityManager::GetUnitAttrs(FUnitHandle Handle)
{
	return IsUnitValid(Handle) ? &Entities.Attributes[Handle.Index] : nullptr;
}

FVector FHktEntityManager::GetUnitLocation(FUnitHandle Handle) const
{
	return IsUnitValid(Handle) ? Entities.Locations[Handle.Index] : FVector::ZeroVector;
}

FRotator FHktEntityManager::GetUnitRotation(FUnitHandle Handle) const
{
	return IsUnitValid(Handle) ? Entities.Rotations[Handle.Index] : FRotator::ZeroRotator;
}

FPlayerHandle FHktEntityManager::GetUnitOwner(FUnitHandle Handle) const
{
	if (IsUnitValid(Handle))
	{
		return FPlayerHandle(Entities.OwnerPlayerIndices[Handle.Index]);
	}
	return FPlayerHandle();
}

void FHktEntityManager::SetVisualActor(FUnitHandle Handle, AActor* Actor)
{
	if (IsUnitValid(Handle)) 
	{
		Entities.VisualActors[Handle.Index] = Actor;
	}
}

AActor* FHktEntityManager::GetVisualActor(FUnitHandle Handle)
{
	return IsUnitValid(Handle) ? Entities.VisualActors[Handle.Index].Get() : nullptr;
}

void FHktEntityManager::SyncLocationFromVisual(FUnitHandle Handle)
{
	if (IsUnitValid(Handle))
	{
		if (AActor* Vis = Entities.VisualActors[Handle.Index].Get())
		{
			FVector OldLocation = Entities.Locations[Handle.Index];
			FVector NewLocation = Vis->GetActorLocation();

			Entities.Locations[Handle.Index] = NewLocation;
			Entities.Rotations[Handle.Index] = Vis->GetActorRotation();

			// Update spatial index
			if (bUseSpatialIndex && SpatialIndex.IsValid())
			{
				SpatialIndex->Update(Handle, OldLocation, NewLocation);
			}
		}
	}
}

void FHktEntityManager::QueryUnitsInSphere(const FVector& Center, float Radius, TArray<FUnitHandle>& OutUnits, bool bFilterActive) const
{
	SCOPE_CYCLE_COUNTER(STAT_SpatialQueries);

	OutUnits.Reset();

	if (bUseSpatialIndex && SpatialIndex.IsValid())
	{
		// Use spatial index for optimized query
		TArray<FUnitHandle> CandidateUnits;
		SpatialIndex->QuerySphere(Center, Radius, CandidateUnits);

		float RadiusSq = Radius * Radius;

		// Filter by actual distance and active status
		for (const FUnitHandle& Handle : CandidateUnits)
		{
			if (!IsUnitValid(Handle))
			{
				continue;
			}

			if (bFilterActive && !Entities.IsActive[Handle.Index])
			{
				continue;
			}

			FVector UnitLocation = Entities.Locations[Handle.Index];
			if (FVector::DistSquared(Center, UnitLocation) <= RadiusSq)
			{
				OutUnits.Add(Handle);
			}
		}
	}
	else
	{
		// Fallback: Linear search through all entities
		float RadiusSq = Radius * Radius;
		
		for (int32 i = 0; i < Entities.Attributes.Num(); ++i)
		{
			if (bFilterActive && !Entities.IsActive[i])
			{
				continue;
			}

			FUnitHandle Handle(i, Entities.Generations[i]);
			if (!IsUnitValid(Handle))
			{
				continue;
			}

			FVector UnitLocation = Entities.Locations[i];
			if (FVector::DistSquared(Center, UnitLocation) <= RadiusSq)
			{
				OutUnits.Add(Handle);
			}
		}
	}
}

void FHktEntityManager::QueryUnitsInBox(const FBox& Box, TArray<FUnitHandle>& OutUnits, bool bFilterActive) const
{
	OutUnits.Reset();

	if (bUseSpatialIndex && SpatialIndex.IsValid())
	{
		// Use spatial index for optimized query
		TArray<FUnitHandle> CandidateUnits;
		SpatialIndex->QueryBox(Box, CandidateUnits);

		// Filter by active status and actual containment
		for (const FUnitHandle& Handle : CandidateUnits)
		{
			if (!IsUnitValid(Handle))
			{
				continue;
			}

			if (bFilterActive && !Entities.IsActive[Handle.Index])
			{
				continue;
			}

			FVector UnitLocation = Entities.Locations[Handle.Index];
			if (Box.IsInside(UnitLocation))
			{
				OutUnits.Add(Handle);
			}
		}
	}
	else
	{
		// Fallback: Linear search
		for (int32 i = 0; i < Entities.Attributes.Num(); ++i)
		{
			if (bFilterActive && !Entities.IsActive[i])
			{
				continue;
			}

			FUnitHandle Handle(i, Entities.Generations[i]);
			if (!IsUnitValid(Handle))
			{
				continue;
			}

			FVector UnitLocation = Entities.Locations[i];
			if (Box.IsInside(UnitLocation))
			{
				OutUnits.Add(Handle);
			}
		}
	}
}

void FHktEntityManager::SetSpatialIndexEnabled(bool bEnabled)
{
	bUseSpatialIndex = bEnabled;
	
	if (!bEnabled && SpatialIndex.IsValid())
	{
		SpatialIndex->Clear();
	}
}
