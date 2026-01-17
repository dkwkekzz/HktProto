// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktEntityManager.h"
#include "GameFramework/Actor.h"

FPlayerHandle FHktEntityManager::AllocPlayer()
{
	int32 Idx = Players.AddPlayer();
	return FPlayerHandle(Idx);
}

FHktAttributeSet* FHktEntityManager::GetPlayerAttrs(FPlayerHandle Handle)
{
	if (Handle.IsValid() && Players.IsActive.IsValidIndex(Handle.Index) && Players.IsActive[Handle.Index])
	{
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
		Entities.VisualActors.Add(nullptr);
	}

	// Initialize
	Entities.IsActive[Idx] = true;
	Entities.Locations[Idx] = SpawnLoc;
	Entities.Rotations[Idx] = SpawnRot;
	Entities.OwnerPlayerIndices[Idx] = Owner.IsValid() ? Owner.Index : INDEX_NONE;
	Entities.Attributes[Idx] = FHktAttributeSet(); // Reset
	Entities.VisualActors[Idx] = nullptr;

	return FUnitHandle(Idx, Entities.Generations[Idx]);
}

void FHktEntityManager::FreeUnit(FUnitHandle Handle)
{
	if (!IsUnitValid(Handle)) return;
	
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
			Entities.Locations[Handle.Index] = Vis->GetActorLocation();
			Entities.Rotations[Handle.Index] = Vis->GetActorRotation();
		}
	}
}
