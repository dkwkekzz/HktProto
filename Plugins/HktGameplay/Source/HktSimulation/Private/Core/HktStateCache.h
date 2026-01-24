// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HktStateTypes.h"

// 전방 선언
struct FHktStateStore;
struct FHktEntityStateStore;

// ============================================================================
// HktStateCache - VM 실행 중 사용하는 로컬 캐시
// 
// 설계 원칙:
// 1. VM 시작 시 필요한 상태를 로컬 캐시로 복사 (Prefetch)
// 2. VM 실행 중에는 로컬 캐시만 읽기/쓰기
// 3. VM 완료 시 변경된 값을 원본에 일괄 적용 (Flush)
// 
// 이점:
// - 캐시 지역성 극대화 (인근 메모리 접근)
// - 원본 상태 변경 최소화
// - 롤백 가능 (Flush 전까지)
// ============================================================================

// 캐시 엔트리 플래그
enum class EHktCacheFlag : uint8
{
    None = 0,
    Loaded = 1 << 0,        // 원본에서 로드됨
    Modified = 1 << 1,      // 수정됨 (Flush 필요)
    Invalid = 1 << 2,       // 무효화됨 (더 이상 사용 불가)
};
ENUM_CLASS_FLAGS(EHktCacheFlag)

// ============================================================================
// FHktEntityCache - 엔티티 상태 로컬 캐시
// 
// 단일 엔티티의 상태를 캐시
// ============================================================================

struct FHktEntityCache
{
    // 원본 참조
    int32 EntityID = INDEX_NONE;
    int32 Generation = 0;
    
    // 캐시된 상태 (값 복사)
    FHktAttributeMap Attributes;
    FHktTagSet StatusTags;
    FHktTagSet BuffTags;
    
    // Transform (자주 접근)
    int32 PosX = 0, PosY = 0, PosZ = 0;
    int32 VelX = 0, VelY = 0, VelZ = 0;
    int32 RotationYaw = 0;
    uint32 Flags = 0;
    
    // 캐시 상태
    EHktCacheFlag CacheFlags = EHktCacheFlag::None;
    
    bool IsLoaded() const { return EnumHasAnyFlags(CacheFlags, EHktCacheFlag::Loaded); }
    bool IsModified() const { return EnumHasAnyFlags(CacheFlags, EHktCacheFlag::Modified); }
    bool IsValid() const { return !EnumHasAnyFlags(CacheFlags, EHktCacheFlag::Invalid); }
    
    void MarkModified() { CacheFlags |= EHktCacheFlag::Modified; }
    
    void Clear()
    {
        EntityID = INDEX_NONE;
        Generation = 0;
        Attributes.Clear();
        StatusTags.Clear();
        BuffTags.Clear();
        PosX = PosY = PosZ = 0;
        VelX = VelY = VelZ = 0;
        RotationYaw = 0;
        Flags = 0;
        CacheFlags = EHktCacheFlag::None;
    }
};

// ============================================================================
// FHktProcessCache - 프로세스 상태 로컬 캐시
// ============================================================================

struct FHktProcessCache
{
    int32 ProcessID = INDEX_NONE;
    FGameplayTag FlowTag;
    
    FHktAttributeMap LocalAttributes;
    FHktTagSet DoingTags;
    FHktTagSet CompletedTags;
    FHktTagSet ContextTags;
    
    int32 Progress = 0;
    
    EHktCacheFlag CacheFlags = EHktCacheFlag::None;
    
    bool IsLoaded() const { return EnumHasAnyFlags(CacheFlags, EHktCacheFlag::Loaded); }
    bool IsModified() const { return EnumHasAnyFlags(CacheFlags, EHktCacheFlag::Modified); }
    
    void MarkModified() { CacheFlags |= EHktCacheFlag::Modified; }
    
    void Clear()
    {
        ProcessID = INDEX_NONE;
        FlowTag = FGameplayTag();
        LocalAttributes.Clear();
        DoingTags.Clear();
        CompletedTags.Clear();
        ContextTags.Clear();
        Progress = 0;
        CacheFlags = EHktCacheFlag::None;
    }
};

// ============================================================================
// FHktStateCache - VM 실행용 통합 로컬 캐시
// 
// 하나의 VM이 실행되는 동안 사용하는 모든 캐시
// ============================================================================

struct FHktStateCache
{
    static constexpr int32 MaxCachedEntities = 16;
    
    // Owner 엔티티 캐시 (R0에 해당)
    FHktEntityCache OwnerCache;
    
    // 추가 엔티티 캐시 (쿼리 결과, 타겟 등)
    FHktEntityCache EntityCaches[MaxCachedEntities];
    int32 EntityCacheCount = 0;
    
    // 프로세스 캐시
    FHktProcessCache ProcessCache;
    
    // 연관된 Store 참조
    FHktStateStore* Store = nullptr;
    
    // ========================================================================
    // 초기화
    // ========================================================================
    
    void Init(FHktStateStore* InStore, int32 ProcessID, int32 OwnerEntityID, int32 OwnerGeneration)
    {
        Store = InStore;
        Clear();
        
        // Owner 로드
        LoadEntity(OwnerEntityID, OwnerGeneration, OwnerCache);
        
        // Process 로드
        LoadProcess(ProcessID);
    }
    
    void Clear()
    {
        OwnerCache.Clear();
        for (int32 i = 0; i < MaxCachedEntities; ++i)
        {
            EntityCaches[i].Clear();
        }
        EntityCacheCount = 0;
        ProcessCache.Clear();
    }
    
    // ========================================================================
    // 엔티티 캐시 관리
    // ========================================================================
    
    // 엔티티 로드 (캐시에 없으면 Store에서 가져옴)
    FHktEntityCache* GetOrLoadEntity(int32 EntityID, int32 Generation)
    {
        // Owner 체크
        if (OwnerCache.EntityID == EntityID && OwnerCache.Generation == Generation)
        {
            return &OwnerCache;
        }
        
        // 기존 캐시 검색
        for (int32 i = 0; i < EntityCacheCount; ++i)
        {
            if (EntityCaches[i].EntityID == EntityID && EntityCaches[i].Generation == Generation)
            {
                return &EntityCaches[i];
            }
        }
        
        // 새로 로드
        if (EntityCacheCount < MaxCachedEntities)
        {
            FHktEntityCache& Cache = EntityCaches[EntityCacheCount];
            if (LoadEntity(EntityID, Generation, Cache))
            {
                EntityCacheCount++;
                return &Cache;
            }
        }
        
        return nullptr;
    }
    
    // 슬롯으로 캐시 접근 (쿼리 결과용)
    FHktEntityCache* GetEntityCacheBySlot(int32 Slot)
    {
        if (Slot < 0 || Slot >= EntityCacheCount) return nullptr;
        return &EntityCaches[Slot];
    }
    
    // ========================================================================
    // Flush - 변경사항 원본에 적용
    // ========================================================================
    
    void Flush();
    
private:
    bool LoadEntity(int32 EntityID, int32 Generation, FHktEntityCache& OutCache);
    void FlushEntity(FHktEntityCache& Cache);
    
    void LoadProcess(int32 ProcessID);
    void FlushProcess();
};

// ============================================================================
// 인라인 구현은 cpp에서
// ============================================================================
