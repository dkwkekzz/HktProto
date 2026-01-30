// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMasterStash.h"
#include "HktVMTypes.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

FHktMasterStash::FHktMasterStash()
    : FHktStashBase()
{
    EntityCreationFrame.SetNumZeroed(MaxEntities);
}

void FHktMasterStash::OnEntityDirty(FHktEntityId Entity)
{
    DirtyEntities.Add(Entity);
}

void FHktMasterStash::ApplyWrites(const TArray<FPendingWrite>& Writes)
{
    for (const auto& W : Writes)
    {
        SetProperty(W.Entity, W.PropertyId, W.Value);
    }
}

bool FHktMasterStash::ValidateEntityFrame(FHktEntityId Entity, int32 FrameNumber) const
{
    if (!IsValidEntity(Entity))
        return false;
    return EntityCreationFrame[Entity] <= FrameNumber;
}

FHktEntitySnapshot FHktMasterStash::CreateEntitySnapshot(FHktEntityId Entity) const
{
    FHktEntitySnapshot Snapshot;
    
    if (!IsValidEntity(Entity))
        return Snapshot;
    
    Snapshot.EntityId = Entity;
    Snapshot.Properties.SetNum(MaxProperties);
    
    for (int32 PropId = 0; PropId < MaxProperties; ++PropId)
    {
        Snapshot.Properties[PropId] = Properties[PropId][Entity];
    }
    
    return Snapshot;
}

TArray<FHktEntitySnapshot> FHktMasterStash::CreateSnapshots(const TArray<FHktEntityId>& Entities) const
{
    TArray<FHktEntitySnapshot> Snapshots;
    Snapshots.Reserve(Entities.Num());
    
    for (FHktEntityId E : Entities)
    {
        FHktEntitySnapshot Snap = CreateEntitySnapshot(E);
        if (Snap.IsValid())
        {
            Snapshots.Add(MoveTemp(Snap));
        }
    }
    
    return Snapshots;
}

TArray<uint8> FHktMasterStash::SerializeFullState() const
{
    TArray<uint8> Data;
    FMemoryWriter Writer(Data);
    
    int32 Frame = CompletedFrameNumber;
    int32 NextId = NextEntityId;
    Writer << Frame;
    Writer << NextId;
    
    // Valid entities
    int32 NumValid = GetEntityCount();
    Writer << NumValid;
    
    for (int32 E = 0; E < MaxEntities; ++E)
    {
        if (ValidEntities[E])
        {
            int32 EntityInt = E;
            Writer << EntityInt;
            for (int32 PropId = 0; PropId < MaxProperties; ++PropId)
            {
                int32 PropValue = Properties[PropId][E];
                Writer << PropValue;
            }
        }
    }
    
    return Data;
}

void FHktMasterStash::DeserializeFullState(const TArray<uint8>& Data)
{
    if (Data.Num() == 0)
        return;
    
    FMemoryReader Reader(Data);
    
    int32 Frame, NextId;
    Reader << Frame;
    Reader << NextId;
    
    CompletedFrameNumber = Frame;
    NextEntityId = FHktEntityId(NextId);
    
    // Clear all
    ValidEntities.Init(false, MaxEntities);
    FreeList.Reset();
    
    int32 NumValid = 0;
    Reader << NumValid;
    
    for (int32 i = 0; i < NumValid; ++i)
    {
        int32 EntityInt;
        Reader << EntityInt;
        
        FHktEntityId E(EntityInt);
        ValidEntities[E] = true;
        for (int32 PropId = 0; PropId < MaxProperties; ++PropId)
        {
            int32 PropValue;
            Reader << PropValue;
            Properties[PropId][E] = PropValue;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("[MasterStash] Deserialized: Frame=%d, Entities=%d"), 
        CompletedFrameNumber, NumValid);
}

bool FHktMasterStash::TryGetPosition(FHktEntityId Entity, FVector& OutPosition) const
{
    if (!IsValidEntity(Entity))
    {
        return false;
    }
    
    OutPosition.X = static_cast<float>(GetProperty(Entity, PropertyId::PosX));
    OutPosition.Y = static_cast<float>(GetProperty(Entity, PropertyId::PosY));
    OutPosition.Z = static_cast<float>(GetProperty(Entity, PropertyId::PosZ));
    
    return true;
}

void FHktMasterStash::SetPosition(FHktEntityId Entity, const FVector& Position)
{
    if (!IsValidEntity(Entity))
    {
        return;
    }
    
    SetProperty(Entity, PropertyId::PosX, FMath::RoundToInt(Position.X));
    SetProperty(Entity, PropertyId::PosY, FMath::RoundToInt(Position.Y));
    SetProperty(Entity, PropertyId::PosZ, FMath::RoundToInt(Position.Z));
}

uint32 FHktMasterStash::CalculatePartialChecksum(const TArray<FHktEntityId>& Entities) const
{
    uint32 Checksum = 0;
    
    for (FHktEntityId E : Entities)
    {
        if (!IsValidEntity(E))
            continue;
        
        for (int32 PropId = 0; PropId < MaxProperties; ++PropId)
        {
            Checksum ^= Properties[PropId][E];
            Checksum = (Checksum << 1) | (Checksum >> 31);
        }
        Checksum ^= E.RawValue;
    }
    
    return Checksum;
}

void FHktMasterStash::ClearDirtyFlags()
{
    DirtyEntities.Reset();
}

void FHktMasterStash::ForEachEntityInRadius(FHktEntityId Center, int32 RadiusCm, TFunctionRef<void(FHktEntityId)> Callback) const
{
    if (!IsValidEntity(Center))
        return;
    
    int32 CX = GetProperty(Center, PropertyId::PosX);
    int32 CY = GetProperty(Center, PropertyId::PosY);
    int32 CZ = GetProperty(Center, PropertyId::PosZ);
    int64 RadiusSq = static_cast<int64>(RadiusCm) * RadiusCm;
    
    ForEachEntity([&](FHktEntityId E)
    {
        if (E == Center) return;
        
        int32 EX = GetProperty(E, PropertyId::PosX);
        int32 EY = GetProperty(E, PropertyId::PosY);
        int32 EZ = GetProperty(E, PropertyId::PosZ);
        
        int64 DX = EX - CX;
        int64 DY = EY - CY;
        int64 DZ = EZ - CZ;
        
        if (DX*DX + DY*DY + DZ*DZ <= RadiusSq)
        {
            Callback(E);
        }
    });
}
