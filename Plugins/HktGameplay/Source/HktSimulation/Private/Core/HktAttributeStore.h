#pragma once

#include "CoreMinimal.h"
#include "HktVMTypes.h"

// ============================================================================
// HktAttributeStore - 영구 속성 SoA 저장소
// 
// 속성은 VM과 독립적으로 영구 저장
// SoA 레이아웃으로 동일 속성 대량 접근 최적화 (예: 모든 엔티티 HP 체크)
// ============================================================================

// 엔티티 슬롯 상태
enum class EHktEntitySlotState : uint8
{
    Free = 0,
    Active,
    PendingDestroy,
};

// ============================================================================
// 속성 저장소
// ============================================================================

struct FHktAttributeStore
{
    static constexpr int32 MaxEntities = 4096;
    
    // ========================================================================
    // 엔티티 관리
    // ========================================================================
    
    alignas(HKT_CACHE_LINE) EHktEntitySlotState SlotStates[MaxEntities];
    alignas(HKT_CACHE_LINE) int32 Generations[MaxEntities];  // 핸들 무효화 감지
    alignas(HKT_CACHE_LINE) FGameplayTag EntityTags[MaxEntities];  // 엔티티 타입
    
    // ========================================================================
    // Core 속성 (자주 접근) - SoA
    // ========================================================================
    
    alignas(HKT_CACHE_LINE) float Health[MaxEntities];
    alignas(HKT_CACHE_LINE) float MaxHealth[MaxEntities];
    alignas(HKT_CACHE_LINE) float Mana[MaxEntities];
    alignas(HKT_CACHE_LINE) float MaxMana[MaxEntities];
    alignas(HKT_CACHE_LINE) float Speed[MaxEntities];
    
    // ========================================================================
    // Transform 속성 - SoA
    // ========================================================================
    
    // Position (X, Y, Z 분리 - SIMD 친화적)
    alignas(HKT_CACHE_LINE) float PositionX[MaxEntities];
    alignas(HKT_CACHE_LINE) float PositionY[MaxEntities];
    alignas(HKT_CACHE_LINE) float PositionZ[MaxEntities];
    
    // Rotation (Yaw만 - 대부분의 경우 충분)
    alignas(HKT_CACHE_LINE) float RotationYaw[MaxEntities];
    
    // Velocity (이동 중인 엔티티)
    alignas(HKT_CACHE_LINE) float VelocityX[MaxEntities];
    alignas(HKT_CACHE_LINE) float VelocityY[MaxEntities];
    alignas(HKT_CACHE_LINE) float VelocityZ[MaxEntities];
    
    // ========================================================================
    // 상태 플래그 (비트마스크)
    // ========================================================================
    
    alignas(HKT_CACHE_LINE) uint32 Flags[MaxEntities];
    
    // Flag bits
    static constexpr uint32 FLAG_ALIVE      = 1 << 0;
    static constexpr uint32 FLAG_MOVING     = 1 << 1;
    static constexpr uint32 FLAG_ATTACKING  = 1 << 2;
    static constexpr uint32 FLAG_STUNNED    = 1 << 3;
    static constexpr uint32 FLAG_BURNING    = 1 << 4;
    static constexpr uint32 FLAG_FROZEN     = 1 << 5;
    static constexpr uint32 FLAG_INVINCIBLE = 1 << 6;
    
    // ========================================================================
    // 이동 목표 (MoveTo용)
    // ========================================================================
    
    alignas(HKT_CACHE_LINE) float TargetX[MaxEntities];
    alignas(HKT_CACHE_LINE) float TargetY[MaxEntities];
    alignas(HKT_CACHE_LINE) float TargetZ[MaxEntities];
    
    // ========================================================================
    // 관리 데이터
    // ========================================================================
    
    int32 ActiveCount = 0;
    int32 HighWaterMark = 0;
    TArray<int32> FreeSlots;
    
    // ========================================================================
    // 초기화
    // ========================================================================
    
    FHktAttributeStore()
    {
        Clear();
    }
    
    void Clear()
    {
        FMemory::Memzero(SlotStates, sizeof(SlotStates));
        FMemory::Memzero(Generations, sizeof(Generations));
        FMemory::Memzero(Health, sizeof(Health));
        FMemory::Memzero(MaxHealth, sizeof(MaxHealth));
        FMemory::Memzero(Mana, sizeof(Mana));
        FMemory::Memzero(MaxMana, sizeof(MaxMana));
        FMemory::Memzero(Speed, sizeof(Speed));
        FMemory::Memzero(PositionX, sizeof(PositionX));
        FMemory::Memzero(PositionY, sizeof(PositionY));
        FMemory::Memzero(PositionZ, sizeof(PositionZ));
        FMemory::Memzero(RotationYaw, sizeof(RotationYaw));
        FMemory::Memzero(VelocityX, sizeof(VelocityX));
        FMemory::Memzero(VelocityY, sizeof(VelocityY));
        FMemory::Memzero(VelocityZ, sizeof(VelocityZ));
        FMemory::Memzero(Flags, sizeof(Flags));
        FMemory::Memzero(TargetX, sizeof(TargetX));
        FMemory::Memzero(TargetY, sizeof(TargetY));
        FMemory::Memzero(TargetZ, sizeof(TargetZ));
        // EntityTags는 POD가 아니므로 별도 초기화
        for (int32 i = 0; i < MaxEntities; ++i)
        {
            EntityTags[i] = FGameplayTag();
        }
        
        ActiveCount = 0;
        HighWaterMark = 0;
        FreeSlots.Empty();
    }
    
    // ========================================================================
    // 엔티티 생성/제거
    // ========================================================================
    
    // 엔티티 생성 (ID, Generation 반환)
    bool CreateEntity(const FGameplayTag& EntityTag, int32& OutID, int32& OutGeneration)
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
            // 엔티티 풀 소진
            return false;
        }
        
        // 슬롯 초기화
        SlotStates[Slot] = EHktEntitySlotState::Active;
        Generations[Slot]++;  // Generation 증가로 이전 핸들 무효화
        EntityTags[Slot] = EntityTag;
        
        // 기본값 설정
        Health[Slot] = 100.0f;
        MaxHealth[Slot] = 100.0f;
        Mana[Slot] = 100.0f;
        MaxMana[Slot] = 100.0f;
        Speed[Slot] = 300.0f;
        Flags[Slot] = FLAG_ALIVE;
        
        // Transform 초기화
        PositionX[Slot] = PositionY[Slot] = PositionZ[Slot] = 0.0f;
        RotationYaw[Slot] = 0.0f;
        VelocityX[Slot] = VelocityY[Slot] = VelocityZ[Slot] = 0.0f;
        TargetX[Slot] = TargetY[Slot] = TargetZ[Slot] = 0.0f;
        
        OutID = Slot;
        OutGeneration = Generations[Slot];
        ActiveCount++;
        
        return true;
    }
    
    // 엔티티 제거
    void DestroyEntity(int32 ID, int32 Generation)
    {
        if (!IsValidEntity(ID, Generation))
        {
            return;
        }
        
        SlotStates[ID] = EHktEntitySlotState::Free;
        Flags[ID] = 0;
        FreeSlots.Add(ID);
        ActiveCount--;
    }
    
    // 핸들 유효성 검사
    FORCEINLINE bool IsValidEntity(int32 ID, int32 Generation) const
    {
        return ID >= 0 && 
               ID < MaxEntities && 
               SlotStates[ID] == EHktEntitySlotState::Active &&
               Generations[ID] == Generation;
    }
    
    // ========================================================================
    // 속성 접근
    // ========================================================================
    
    // Position 가져오기
    FORCEINLINE FVector GetPosition(int32 ID) const
    {
        return FVector(PositionX[ID], PositionY[ID], PositionZ[ID]);
    }
    
    // Position 설정
    FORCEINLINE void SetPosition(int32 ID, const FVector& Pos)
    {
        PositionX[ID] = static_cast<float>(Pos.X);
        PositionY[ID] = static_cast<float>(Pos.Y);
        PositionZ[ID] = static_cast<float>(Pos.Z);
    }
    
    // Velocity 가져오기
    FORCEINLINE FVector GetVelocity(int32 ID) const
    {
        return FVector(VelocityX[ID], VelocityY[ID], VelocityZ[ID]);
    }
    
    // Velocity 설정
    FORCEINLINE void SetVelocity(int32 ID, const FVector& Vel)
    {
        VelocityX[ID] = static_cast<float>(Vel.X);
        VelocityY[ID] = static_cast<float>(Vel.Y);
        VelocityZ[ID] = static_cast<float>(Vel.Z);
    }
    
    // Target 가져오기
    FORCEINLINE FVector GetTarget(int32 ID) const
    {
        return FVector(TargetX[ID], TargetY[ID], TargetZ[ID]);
    }
    
    // Target 설정
    FORCEINLINE void SetTarget(int32 ID, const FVector& Target)
    {
        TargetX[ID] = static_cast<float>(Target.X);
        TargetY[ID] = static_cast<float>(Target.Y);
        TargetZ[ID] = static_cast<float>(Target.Z);
    }
    
    // 플래그 조작
    FORCEINLINE bool HasFlag(int32 ID, uint32 Flag) const { return (Flags[ID] & Flag) != 0; }
    FORCEINLINE void SetFlag(int32 ID, uint32 Flag) { Flags[ID] |= Flag; }
    FORCEINLINE void ClearFlag(int32 ID, uint32 Flag) { Flags[ID] &= ~Flag; }
    
    // ========================================================================
    // 공간 쿼리
    // ========================================================================
    
    // 반경 내 엔티티 쿼리 (결과를 OutResults에 추가)
    void QueryRadius(const FVector& Origin, float Radius, TArray<int32>& OutResults, int32 MaxResults = 64) const
    {
        const float RadiusSq = Radius * Radius;
        
        for (int32 i = 0; i < HighWaterMark && OutResults.Num() < MaxResults; ++i)
        {
            if (SlotStates[i] != EHktEntitySlotState::Active)
            {
                continue;
            }
            
            const float DX = PositionX[i] - static_cast<float>(Origin.X);
            const float DY = PositionY[i] - static_cast<float>(Origin.Y);
            const float DZ = PositionZ[i] - static_cast<float>(Origin.Z);
            const float DistSq = DX*DX + DY*DY + DZ*DZ;
            
            if (DistSq <= RadiusSq)
            {
                OutResults.Add(i);
            }
        }
    }
    
    // ========================================================================
    // Tick 처리 (이동 등)
    // ========================================================================
    
    void TickMovement(float DeltaTime)
    {
        for (int32 i = 0; i < HighWaterMark; ++i)
        {
            if (SlotStates[i] != EHktEntitySlotState::Active)
            {
                continue;
            }
            
            if (HasFlag(i, FLAG_MOVING))
            {
                // 위치 업데이트
                PositionX[i] += VelocityX[i] * DeltaTime;
                PositionY[i] += VelocityY[i] * DeltaTime;
                PositionZ[i] += VelocityZ[i] * DeltaTime;
            }
        }
    }
    
    // 목적지 도착 체크
    bool HasArrivedAtTarget(int32 ID, float Tolerance = 10.0f) const
    {
        if (!HasFlag(ID, FLAG_MOVING))
        {
            return true;
        }
        
        const float DX = TargetX[ID] - PositionX[ID];
        const float DY = TargetY[ID] - PositionY[ID];
        const float DZ = TargetZ[ID] - PositionZ[ID];
        const float DistSq = DX*DX + DY*DY + DZ*DZ;
        
        return DistSq <= Tolerance * Tolerance;
    }
};
