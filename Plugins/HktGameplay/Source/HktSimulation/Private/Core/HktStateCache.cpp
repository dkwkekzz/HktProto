// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktStateCache.h"
#include "HktStateStore.h"

// ============================================================================
// FHktStateCache 구현
// ============================================================================

void FHktStateCache::Flush()
{
    if (!Store) return;
    
    // Owner 엔티티 Flush
    if (OwnerCache.IsModified())
    {
        FlushEntity(OwnerCache);
    }
    
    // 추가 엔티티 Flush
    for (int32 i = 0; i < EntityCacheCount; ++i)
    {
        if (EntityCaches[i].IsModified())
        {
            FlushEntity(EntityCaches[i]);
        }
    }
    
    // Process Flush
    if (ProcessCache.IsModified())
    {
        FlushProcess();
    }
}

bool FHktStateCache::LoadEntity(int32 EntityID, int32 Generation, FHktEntityCache& OutCache)
{
    if (!Store) return false;
    
    const FHktEntityState* State = Store->Entities.Get(EntityID);
    if (!State || State->Generation != Generation)
    {
        OutCache.CacheFlags = EHktCacheFlag::Invalid;
        return false;
    }
    
    OutCache.Clear();
    OutCache.EntityID = EntityID;
    OutCache.Generation = Generation;
    
    // 속성 복사
    OutCache.Attributes = State->Attributes;
    OutCache.StatusTags = State->StatusTags;
    OutCache.BuffTags = State->BuffTags;
    
    // Transform 복사
    OutCache.PosX = State->PosX;
    OutCache.PosY = State->PosY;
    OutCache.PosZ = State->PosZ;
    OutCache.VelX = State->VelX;
    OutCache.VelY = State->VelY;
    OutCache.VelZ = State->VelZ;
    OutCache.RotationYaw = State->RotationYaw;
    OutCache.Flags = State->Flags;
    
    OutCache.CacheFlags = EHktCacheFlag::Loaded;
    return true;
}

void FHktStateCache::FlushEntity(FHktEntityCache& Cache)
{
    if (!Store) return;
    if (!Cache.IsLoaded() || !Cache.IsModified()) return;
    
    FHktEntityState* State = Store->Entities.Get(Cache.EntityID);
    if (!State || State->Generation != Cache.Generation)
    {
        // 원본이 이미 제거되었거나 Generation 불일치
        Cache.CacheFlags |= EHktCacheFlag::Invalid;
        return;
    }
    
    // 속성 적용
    State->Attributes = Cache.Attributes;
    State->StatusTags = Cache.StatusTags;
    State->BuffTags = Cache.BuffTags;
    
    // Transform 적용
    State->PosX = Cache.PosX;
    State->PosY = Cache.PosY;
    State->PosZ = Cache.PosZ;
    State->VelX = Cache.VelX;
    State->VelY = Cache.VelY;
    State->VelZ = Cache.VelZ;
    State->RotationYaw = Cache.RotationYaw;
    State->Flags = Cache.Flags;
    
    // Modified 플래그 클리어
    Cache.CacheFlags &= ~EHktCacheFlag::Modified;
}

void FHktStateCache::LoadProcess(int32 ProcessID)
{
    if (!Store) return;
    
    const FHktProcessState* State = Store->Processes.Get(ProcessID);
    if (!State)
    {
        ProcessCache.Clear();
        return;
    }
    
    ProcessCache.ProcessID = ProcessID;
    ProcessCache.FlowTag = State->FlowTag;
    ProcessCache.LocalAttributes = State->LocalAttributes;
    ProcessCache.DoingTags = State->DoingTags;
    ProcessCache.CompletedTags = State->CompletedTags;
    ProcessCache.ContextTags = State->ContextTags;
    ProcessCache.Progress = State->Progress;
    ProcessCache.CacheFlags = EHktCacheFlag::Loaded;
}

void FHktStateCache::FlushProcess()
{
    if (!Store) return;
    if (!ProcessCache.IsLoaded() || !ProcessCache.IsModified()) return;
    
    FHktProcessState* State = Store->Processes.Get(ProcessCache.ProcessID);
    if (!State) return;
    
    State->LocalAttributes = ProcessCache.LocalAttributes;
    State->DoingTags = ProcessCache.DoingTags;
    State->CompletedTags = ProcessCache.CompletedTags;
    State->ContextTags = ProcessCache.ContextTags;
    State->Progress = ProcessCache.Progress;
    
    ProcessCache.CacheFlags &= ~EHktCacheFlag::Modified;
}
