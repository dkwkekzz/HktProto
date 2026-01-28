#include "VMStoreInterface.h"
#include "MasterStash.h"
#include "VisibleStash.h"

// ============================================================================
// FMasterStashWrapper
// ============================================================================

bool FMasterStashWrapper::IsValidEntity(EntityId Entity) const
{
    return Stash ? Stash->IsValidEntity(Entity) : false;
}

int32 FMasterStashWrapper::GetProperty(EntityId Entity, uint16 PropertyId) const
{
    return Stash ? Stash->GetProperty(Entity, PropertyId) : 0;
}

void FMasterStashWrapper::SetProperty(EntityId Entity, uint16 PropertyId, int32 Value)
{
    if (Stash) Stash->SetProperty(Entity, PropertyId, Value);
}

EntityId FMasterStashWrapper::AllocateEntity()
{
    return Stash ? Stash->AllocateEntity() : InvalidEntityId;
}

void FMasterStashWrapper::FreeEntity(EntityId Entity)
{
    if (Stash) Stash->FreeEntity(Entity);
}

// ============================================================================
// FVisibleStashWrapper
// ============================================================================

bool FVisibleStashWrapper::IsValidEntity(EntityId Entity) const
{
    return Stash ? Stash->IsValidEntity(Entity) : false;
}

int32 FVisibleStashWrapper::GetProperty(EntityId Entity, uint16 PropertyId) const
{
    return Stash ? Stash->GetProperty(Entity, PropertyId) : 0;
}

void FVisibleStashWrapper::SetProperty(EntityId Entity, uint16 PropertyId, int32 Value)
{
    if (Stash) Stash->SetProperty(Entity, PropertyId, Value);
}

EntityId FVisibleStashWrapper::AllocateEntity()
{
    return Stash ? Stash->AllocateEntity() : InvalidEntityId;
}

void FVisibleStashWrapper::FreeEntity(EntityId Entity)
{
    if (Stash) Stash->FreeEntity(Entity);
}

// ============================================================================
// FVMStore
// ============================================================================

int32 FVMStore::Read(uint16 PropertyId) const
{
    return ReadEntity(SourceEntity, PropertyId);
}

int32 FVMStore::ReadEntity(EntityId Entity, uint16 PropertyId) const
{
    uint64 Key = MakeCacheKey(Entity, PropertyId);
    if (const int32* Cached = LocalCache.Find(Key))
    {
        return *Cached;
    }
    
    return Stash ? Stash->GetProperty(Entity, PropertyId) : 0;
}

void FVMStore::Write(uint16 PropertyId, int32 Value)
{
    WriteEntity(SourceEntity, PropertyId, Value);
}

void FVMStore::WriteEntity(EntityId Entity, uint16 PropertyId, int32 Value)
{
    uint64 Key = MakeCacheKey(Entity, PropertyId);
    LocalCache.Add(Key, Value);
    
    FPendingWrite W;
    W.Entity = Entity;
    W.PropertyId = PropertyId;
    W.Value = Value;
    PendingWrites.Add(W);
}

void FVMStore::ClearPendingWrites()
{
    PendingWrites.Reset();
}

void FVMStore::Reset()
{
    PendingWrites.Reset();
    LocalCache.Reset();
    SourceEntity = InvalidEntityId;
    TargetEntity = InvalidEntityId;
}