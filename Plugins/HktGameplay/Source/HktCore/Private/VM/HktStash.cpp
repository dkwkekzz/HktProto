// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktStash.h"

FHktStashBase::FHktStashBase()
{
    Properties.SetNum(MaxProperties);
    for (int32 i = 0; i < MaxProperties; ++i)
    {
        Properties[i].SetNumZeroed(MaxEntities);
    }
    
    ValidEntities.Init(false, MaxEntities);
}

FHktEntityId FHktStashBase::AllocateEntity()
{
    FHktEntityId Id;
    
    if (FreeList.Num() > 0)
    {
        Id = FreeList.Pop();
    }
    else if (NextEntityId < MaxEntities)
    {
        Id = NextEntityId++;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[Stash] Entity limit reached!"));
        return InvalidEntityId;
    }
    
    ValidEntities[Id] = true;
    
    // 속성 초기화
    for (int32 PropId = 0; PropId < MaxProperties; ++PropId)
    {
        Properties[PropId][Id] = 0;
    }
    
    OnEntityDirty(Id);
    
    UE_LOG(LogTemp, Verbose, TEXT("[Stash] Entity %d allocated"), Id.RawValue);
    return Id;
}

void FHktStashBase::FreeEntity(FHktEntityId Entity)
{
    if (Entity < MaxEntities && ValidEntities[Entity])
    {
        ValidEntities[Entity] = false;
        FreeList.Add(Entity);
        OnEntityDirty(Entity);
        
        UE_LOG(LogTemp, Verbose, TEXT("[Stash] Entity %d freed"), Entity.RawValue);
    }
}

bool FHktStashBase::IsValidEntity(FHktEntityId Entity) const
{
    return Entity < MaxEntities && ValidEntities[Entity];
}

int32 FHktStashBase::GetProperty(FHktEntityId Entity, uint16 PropertyId) const
{
    if (!IsValidEntity(Entity) || PropertyId >= MaxProperties)
        return 0;
    return Properties[PropertyId][Entity];
}

void FHktStashBase::SetProperty(FHktEntityId Entity, uint16 PropertyId, int32 Value)
{
    if (Entity >= MaxEntities || PropertyId >= MaxProperties)
        return;
    
    // 자동 생성 모드 (VisibleStash용)
    if (bAutoCreateOnSet && !ValidEntities[Entity])
    {
        ValidEntities[Entity] = true;
        if (Entity >= NextEntityId)
            NextEntityId = Entity + 1;
        
        // 속성 초기화
        for (int32 PropId = 0; PropId < MaxProperties; ++PropId)
        {
            Properties[PropId][Entity] = 0;
        }
    }
    
    if (!ValidEntities[Entity])
        return;
    
    if (Properties[PropertyId][Entity] != Value)
    {
        Properties[PropertyId][Entity] = Value;
        OnEntityDirty(Entity);
    }
}

int32 FHktStashBase::GetEntityCount() const
{
    int32 Count = 0;
    for (int32 i = 0; i < MaxEntities; ++i)
    {
        if (ValidEntities[i])
            Count++;
    }
    return Count;
}

void FHktStashBase::MarkFrameCompleted(int32 FrameNumber)
{
    CompletedFrameNumber = FrameNumber;
}

void FHktStashBase::ForEachEntity(TFunctionRef<void(FHktEntityId)> Callback) const
{
    for (int32 E = 0; E < MaxEntities; ++E)
    {
        if (ValidEntities[E])
        {
            Callback(FHktEntityId(E));
        }
    }
}

uint32 FHktStashBase::CalculateChecksum() const
{
    uint32 Checksum = 0;
    
    ForEachEntity([&](FHktEntityId E)
    {
        for (int32 PropId = 0; PropId < MaxProperties; ++PropId)
        {
            Checksum ^= Properties[PropId][E];
            Checksum = (Checksum << 1) | (Checksum >> 31);
        }
        Checksum ^= E.RawValue;
    });
    
    Checksum ^= CompletedFrameNumber;
    return Checksum;
}
