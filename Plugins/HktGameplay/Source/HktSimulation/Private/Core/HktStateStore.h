// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HktStateTypes.h"

// ============================================================================
// HktStateStore - 상태 저장소
// 
// 모든 상태의 중앙 저장소
// 직접 접근하지 않고 StateCache를 통해 배치 읽기/쓰기
// ============================================================================

// ============================================================================
// FHktEntityStateStore - 엔티티 상태 저장소
// ============================================================================

struct FHktEntityStateStore
{
    static constexpr int32 MaxEntities = 4096;
    
    // 상태 배열
    FHktEntityState States[MaxEntities];
    
    // Free list
    TArray<int32> FreeSlots;
    int32 HighWaterMark = 0;
    int32 ActiveCount = 0;
    
    // ========================================================================
    // 엔티티 관리
    // ========================================================================
    
    int32 Allocate(const FGameplayTag& EntityType)
    {
        int32 Slot = INDEX_NONE;
        
        if (FreeSlots.Num() > 0)
        {
            Slot = FreeSlots.Pop(EAllowShrinking::No);
        }
        else if (HighWaterMark < MaxEntities)
        {
            Slot = HighWaterMark++;
        }
        else
        {
            return INDEX_NONE;
        }
        
        FHktEntityState& State = States[Slot];
        State.Reset();
        State.EntityID = Slot;
        State.Generation++;
        State.EntityType = EntityType;
        State.Flags = FHktEntityState::FLAG_ACTIVE | FHktEntityState::FLAG_ALIVE;
        
        // 기본 속성 설정
        State.Attributes.SetFloat(HktAttrKeys::Health, 100.0f);
        State.Attributes.SetFloat(HktAttrKeys::MaxHealth, 100.0f);
        State.Attributes.SetFloat(HktAttrKeys::Speed, 300.0f);
        
        ActiveCount++;
        return Slot;
    }
    
    void Free(int32 Slot)
    {
        if (Slot < 0 || Slot >= MaxEntities) return;
        if (!(States[Slot].Flags & FHktEntityState::FLAG_ACTIVE)) return;
        
        States[Slot].Flags = 0;
        States[Slot].EntityID = INDEX_NONE;
        FreeSlots.Add(Slot);
        ActiveCount--;
    }
    
    bool IsValid(int32 ID, int32 Generation) const
    {
        if (ID < 0 || ID >= MaxEntities) return false;
        const FHktEntityState& S = States[ID];
        return (S.Flags & FHktEntityState::FLAG_ACTIVE) && S.Generation == Generation;
    }
    
    FHktEntityState* Get(int32 ID)
    {
        if (ID < 0 || ID >= MaxEntities) return nullptr;
        FHktEntityState& S = States[ID];
        if (!(S.Flags & FHktEntityState::FLAG_ACTIVE)) return nullptr;
        return &S;
    }
    
    const FHktEntityState* Get(int32 ID) const
    {
        if (ID < 0 || ID >= MaxEntities) return nullptr;
        const FHktEntityState& S = States[ID];
        if (!(S.Flags & FHktEntityState::FLAG_ACTIVE)) return nullptr;
        return &S;
    }
    
    // ========================================================================
    // 공간 쿼리
    // ========================================================================
    
    void QueryRadius(const FVector& Origin, float Radius, TArray<int32>& OutResults, int32 MaxResults = 64) const
    {
        const int32 RadiusSqFixed = HktFloatToFixed(Radius * Radius);
        const int32 OriginX = HktFloatToFixed(Origin.X);
        const int32 OriginY = HktFloatToFixed(Origin.Y);
        const int32 OriginZ = HktFloatToFixed(Origin.Z);
        
        for (int32 i = 0; i < HighWaterMark && OutResults.Num() < MaxResults; ++i)
        {
            const FHktEntityState& S = States[i];
            if (!(S.Flags & FHktEntityState::FLAG_ACTIVE)) continue;
            
            // Fixed point 거리 계산 (overflow 주의)
            int64 DX = static_cast<int64>(S.PosX - OriginX);
            int64 DY = static_cast<int64>(S.PosY - OriginY);
            int64 DZ = static_cast<int64>(S.PosZ - OriginZ);
            int64 DistSq = (DX*DX + DY*DY + DZ*DZ) / (HKT_FIXED_SCALE * HKT_FIXED_SCALE);
            
            if (DistSq <= RadiusSqFixed)
            {
                OutResults.Add(i);
            }
        }
    }
    
    // ========================================================================
    // 태그 기반 쿼리
    // ========================================================================
    
    void QueryByTag(FGameplayTag Tag, TArray<int32>& OutResults, int32 MaxResults = 64) const
    {
        for (int32 i = 0; i < HighWaterMark && OutResults.Num() < MaxResults; ++i)
        {
            const FHktEntityState& S = States[i];
            if (!(S.Flags & FHktEntityState::FLAG_ACTIVE)) continue;
            
            if (S.StatusTags.HasTag(Tag) || S.BuffTags.HasTag(Tag) || S.EntityType.MatchesTag(Tag))
            {
                OutResults.Add(i);
            }
        }
    }
    
    // ========================================================================
    // Tick
    // ========================================================================
    
    void TickMovement(float DeltaTime)
    {
        const int32 DeltaFixed = HktFloatToFixed(DeltaTime);
        
        for (int32 i = 0; i < HighWaterMark; ++i)
        {
            FHktEntityState& S = States[i];
            if (!(S.Flags & FHktEntityState::FLAG_ACTIVE)) continue;
            if (!(S.Flags & FHktEntityState::FLAG_MOVING)) continue;
            
            // 속도 적용 (Fixed point 곱셈)
            S.PosX += (S.VelX * DeltaFixed) / HKT_FIXED_SCALE;
            S.PosY += (S.VelY * DeltaFixed) / HKT_FIXED_SCALE;
            S.PosZ += (S.VelZ * DeltaFixed) / HKT_FIXED_SCALE;
        }
    }
    
    void Clear()
    {
        for (int32 i = 0; i < MaxEntities; ++i)
        {
            States[i].Reset();
        }
        FreeSlots.Empty();
        HighWaterMark = 0;
        ActiveCount = 0;
    }
};

// ============================================================================
// FHktPlayerStateStore - 플레이어 상태 저장소
// ============================================================================

struct FHktPlayerStateStore
{
    static constexpr int32 MaxPlayers = 64;
    
    FHktPlayerState States[MaxPlayers];
    int32 ActiveCount = 0;
    
    FHktPlayerState* Get(int32 PlayerID)
    {
        if (PlayerID < 0 || PlayerID >= MaxPlayers) return nullptr;
        return States[PlayerID].IsValid() ? &States[PlayerID] : nullptr;
    }
    
    const FHktPlayerState* Get(int32 PlayerID) const
    {
        if (PlayerID < 0 || PlayerID >= MaxPlayers) return nullptr;
        return States[PlayerID].IsValid() ? &States[PlayerID] : nullptr;
    }
    
    int32 Allocate()
    {
        for (int32 i = 0; i < MaxPlayers; ++i)
        {
            if (!States[i].IsValid())
            {
                States[i].Reset();
                States[i].PlayerID = i;
                ActiveCount++;
                return i;
            }
        }
        return INDEX_NONE;
    }
    
    void Free(int32 PlayerID)
    {
        if (PlayerID < 0 || PlayerID >= MaxPlayers) return;
        if (!States[PlayerID].IsValid()) return;
        
        States[PlayerID].Reset();
        ActiveCount--;
    }
    
    void Clear()
    {
        for (int32 i = 0; i < MaxPlayers; ++i)
        {
            States[i].Reset();
        }
        ActiveCount = 0;
    }
};

// ============================================================================
// FHktProcessStateStore - 프로세스 상태 저장소
// ============================================================================

struct FHktProcessStateStore
{
    static constexpr int32 MaxProcesses = HKT_VM_BATCH_SIZE;
    
    FHktProcessState States[MaxProcesses];
    
    FHktProcessState* Get(int32 ProcessID)
    {
        if (ProcessID < 0 || ProcessID >= MaxProcesses) return nullptr;
        return &States[ProcessID];
    }
    
    const FHktProcessState* Get(int32 ProcessID) const
    {
        if (ProcessID < 0 || ProcessID >= MaxProcesses) return nullptr;
        return &States[ProcessID];
    }
    
    void Init(int32 ProcessID, const FGameplayTag& FlowTag)
    {
        if (ProcessID < 0 || ProcessID >= MaxProcesses) return;
        States[ProcessID].Reset();
        States[ProcessID].ProcessID = ProcessID;
        States[ProcessID].FlowTag = FlowTag;
    }
    
    void Clear(int32 ProcessID)
    {
        if (ProcessID < 0 || ProcessID >= MaxProcesses) return;
        States[ProcessID].Reset();
    }
    
    void ClearAll()
    {
        for (int32 i = 0; i < MaxProcesses; ++i)
        {
            States[i].Reset();
        }
    }
};

// ============================================================================
// FHktStateStore - 통합 상태 저장소
// ============================================================================

struct FHktStateStore
{
    FHktEntityStateStore Entities;
    FHktPlayerStateStore Players;
    FHktProcessStateStore Processes;
    
    void Clear()
    {
        Entities.Clear();
        Players.Clear();
        Processes.ClearAll();
    }
    
    void Tick(float DeltaTime)
    {
        Entities.TickMovement(DeltaTime);
    }
};
