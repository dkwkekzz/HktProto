#include "VMStore.h"

// ============================================================================
// FVMStore
// ============================================================================

int32 FVMStore::Read(uint16 PropertyId) const
{
    return ReadEntity(SourceEntity, PropertyId);
}

int32 FVMStore::ReadEntity(EntityId Entity, uint16 PropertyId) const
{
    // 1. 로컬 캐시 먼저 확인 (VM 내에서 쓴 값 우선)
    uint64 Key = MakeCacheKey(Entity, PropertyId);
    if (const int32* Cached = LocalCache.Find(Key))
    {
        return *Cached;
    }
    
    // 2. Stash에서 읽기
    if (!Stash) return 0;
    return Stash->GetProperty(Entity, PropertyId);
}

float FVMStore::ReadFloat(uint16 PropertyId) const
{
    int32 Raw = Read(PropertyId);
    return *reinterpret_cast<const float*>(&Raw);
}

void FVMStore::Write(uint16 PropertyId, int32 Value)
{
    WriteEntity(SourceEntity, PropertyId, Value);
}

void FVMStore::WriteEntity(EntityId Entity, uint16 PropertyId, int32 Value)
{
    // 1. 로컬 캐시에 기록 (VM 내 일관성)
    uint64 Key = MakeCacheKey(Entity, PropertyId);
    LocalCache.Add(Key, Value);
    
    // 2. PendingWrites에 기록 (VM 완료 시 Stash에 적용)
    FPendingWrite W;
    W.Entity = Entity;
    W.PropertyId = PropertyId;
    W.Value = Value;
    PendingWrites.Add(W);
}

void FVMStore::WriteFloat(uint16 PropertyId, float Value)
{
    Write(PropertyId, *reinterpret_cast<const int32*>(&Value));
}

void FVMStore::ClearPendingWrites()
{
    PendingWrites.Reset();
    // 로컬 캐시는 유지 (VM이 아직 실행 중일 수 있음)
}

void FVMStore::Reset()
{
    PendingWrites.Reset();
    LocalCache.Reset();
    SourceEntity = InvalidEntityId;
    TargetEntity = InvalidEntityId;
}

// ============================================================================
// FStashBase
// ============================================================================

void FStashBase::Initialize()
{
    Properties.SetNum(PropertyId::Max);
    for (int32 i = 0; i < PropertyId::Max; ++i)
    {
        Properties[i].SetNumZeroed(MaxEntities);
    }
    
    ValidEntities.Init(false, MaxEntities);
    EntityCreationFrame.SetNumZeroed(MaxEntities);
    NextEntityId = 0;
    CompletedFrameNumber = 0;
    FreeList.Reset();
}

EntityId FStashBase::AllocateEntity()
{
    EntityId Id;
    
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
        return InvalidEntityId;
    }
    
    ValidEntities[Id] = true;
    EntityCreationFrame[Id] = CompletedFrameNumber;
    
    for (int32 PropId = 0; PropId < PropertyId::Max; ++PropId)
    {
        Properties[PropId][Id] = 0;
    }
    
    return Id;
}

void FStashBase::FreeEntity(EntityId Entity)
{
    if (Entity < MaxEntities && ValidEntities[Entity])
    {
        ValidEntities[Entity] = false;
        FreeList.Add(Entity);
    }
}

bool FStashBase::IsValidEntity(EntityId Entity) const
{
    return Entity < MaxEntities && ValidEntities[Entity];
}

int32 FStashBase::GetProperty(EntityId Entity, uint16 PropertyId) const
{
    if (!IsValidEntity(Entity) || PropertyId >= PropertyId::Max)
        return 0;
    return Properties[PropertyId][Entity];
}

void FStashBase::SetProperty(EntityId Entity, uint16 PropertyId, int32 Value)
{
    if (IsValidEntity(Entity) && PropertyId < PropertyId::Max)
    {
        Properties[PropertyId][Entity] = Value;
    }
}

void FStashBase::ApplyWrites(const TArray<FVMStore::FPendingWrite>& Writes)
{
    for (const auto& W : Writes)
    {
        SetProperty(W.Entity, W.PropertyId, W.Value);
    }
}

void FStashBase::MarkFrameCompleted(int32 FrameNumber)
{
    CompletedFrameNumber = FrameNumber;
}

bool FStashBase::ValidateEntityFrame(EntityId Entity, int32 FrameNumber) const
{
    if (!IsValidEntity(Entity))
        return false;
    return EntityCreationFrame[Entity] <= FrameNumber;
}

int32 FStashBase::GetEntityCount() const
{
    int32 Count = 0;
    for (int32 i = 0; i < MaxEntities; ++i)
    {
        if (ValidEntities[i])
            Count++;
    }
    return Count;
}

void FStashBase::SerializeState(FArchive& Ar)
{
    Ar << CompletedFrameNumber;
    Ar << NextEntityId;
    
    int32 NumEntities = MaxEntities;
    Ar << NumEntities;
    
    if (Ar.IsLoading())
    {
        ValidEntities.Init(false, NumEntities);
    }
    
    for (int32 i = 0; i < NumEntities; ++i)
    {
        bool bValid = ValidEntities[i];
        Ar << bValid;
        if (Ar.IsLoading())
            ValidEntities[i] = bValid;
    }
    
    for (int32 PropId = 0; PropId < PropertyId::Max; ++PropId)
    {
        Ar << Properties[PropId];
    }
    
    Ar << EntityCreationFrame;
    Ar << FreeList;
}

uint32 FStashBase::CalculateChecksum() const
{
    uint32 Checksum = 0;
    
    for (int32 PropId = 0; PropId < PropertyId::Max; ++PropId)
    {
        for (int32 Idx = 0; Idx < MaxEntities; ++Idx)
        {
            if (ValidEntities[Idx])
            {
                Checksum ^= Properties[PropId][Idx];
                Checksum = (Checksum << 1) | (Checksum >> 31);
            }
        }
    }
    
    Checksum ^= CompletedFrameNumber;
    return Checksum;
}