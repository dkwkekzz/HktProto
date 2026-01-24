// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HktVMTypes.h"

// ============================================================================
// HktState Types - 확장 가능한 상태 시스템
// 
// 설계 원칙:
// 1. 모든 속성값은 int32 기반 (float는 Fixed Point로 변환)
// 2. 태그로 추상적 상태 표현 (무엇을, 어떻게)
// 3. 캐시 효율: 로컬 캐시 → 일괄 적용
// ============================================================================

// Fixed Point 변환 (float → int32, 소수점 2자리 정밀도)
constexpr int32 HKT_FIXED_SCALE = 100;

FORCEINLINE int32 HktFloatToFixed(float Value) { return FMath::RoundToInt(Value * HKT_FIXED_SCALE); }
FORCEINLINE float HktFixedToFloat(int32 Value) { return static_cast<float>(Value) / HKT_FIXED_SCALE; }

// ============================================================================
// FHktAttributeKey - 속성 식별자
// 
// GameplayTag 기반으로 무한 확장 가능
// 예: "Attr.Health", "Attr.Mana", "Attr.Speed", "Attr.Custom.MyAttr"
// ============================================================================

using FHktAttributeKey = FGameplayTag;

// 사전 정의된 속성 키 (편의용, 실제로는 태그로 접근)
namespace HktAttrKeys
{
    // 런타임에 초기화됨
    inline FGameplayTag Health;
    inline FGameplayTag MaxHealth;
    inline FGameplayTag Mana;
    inline FGameplayTag MaxMana;
    inline FGameplayTag Speed;
    inline FGameplayTag Armor;
    inline FGameplayTag Attack;
    
    // 초기화 함수
    inline void Initialize()
    {
        Health = FGameplayTag::RequestGameplayTag(FName("Attr.Health"), false);
        MaxHealth = FGameplayTag::RequestGameplayTag(FName("Attr.MaxHealth"), false);
        Mana = FGameplayTag::RequestGameplayTag(FName("Attr.Mana"), false);
        MaxMana = FGameplayTag::RequestGameplayTag(FName("Attr.MaxMana"), false);
        Speed = FGameplayTag::RequestGameplayTag(FName("Attr.Speed"), false);
        Armor = FGameplayTag::RequestGameplayTag(FName("Attr.Armor"), false);
        Attack = FGameplayTag::RequestGameplayTag(FName("Attr.Attack"), false);
    }
}

// ============================================================================
// FHktAttribute - 속성 단위
// 
// int32 기반, 태그로 식별
// ============================================================================

struct FHktAttribute
{
    FHktAttributeKey Key;       // 속성 식별 태그
    int32 Value = 0;            // 값 (Fixed Point)
    
    FHktAttribute() = default;
    FHktAttribute(FHktAttributeKey InKey, int32 InValue) : Key(InKey), Value(InValue) {}
    FHktAttribute(FHktAttributeKey InKey, float InValue) : Key(InKey), Value(HktFloatToFixed(InValue)) {}
    
    float AsFloat() const { return HktFixedToFloat(Value); }
    void SetFloat(float V) { Value = HktFloatToFixed(V); }
};

// ============================================================================
// FHktAttributeMap - 속성 맵 (동적 속성 집합)
// 
// 태그 → 값 매핑
// ============================================================================

struct FHktAttributeMap
{
    TMap<FHktAttributeKey, int32> Values;
    
    // 속성 접근
    FORCEINLINE int32 Get(FHktAttributeKey Key, int32 Default = 0) const
    {
        const int32* Found = Values.Find(Key);
        return Found ? *Found : Default;
    }
    
    FORCEINLINE float GetFloat(FHktAttributeKey Key, float Default = 0.0f) const
    {
        return HktFixedToFloat(Get(Key, HktFloatToFixed(Default)));
    }
    
    FORCEINLINE void Set(FHktAttributeKey Key, int32 Value)
    {
        Values.FindOrAdd(Key) = Value;
    }
    
    FORCEINLINE void SetFloat(FHktAttributeKey Key, float Value)
    {
        Set(Key, HktFloatToFixed(Value));
    }
    
    FORCEINLINE void Add(FHktAttributeKey Key, int32 Delta)
    {
        Values.FindOrAdd(Key) += Delta;
    }
    
    FORCEINLINE bool Has(FHktAttributeKey Key) const
    {
        return Values.Contains(Key);
    }
    
    FORCEINLINE void Remove(FHktAttributeKey Key)
    {
        Values.Remove(Key);
    }
    
    void Clear() { Values.Empty(); }
    int32 Num() const { return Values.Num(); }
};

// ============================================================================
// FHktTagSet - 태그 집합 (추상적 상태 표현)
// 
// "무엇을" / "어떻게" 표현
// 예: "Status.Burning", "Status.Stunned", "Action.Casting", "Buff.Shield"
// ============================================================================

struct FHktTagSet
{
    FGameplayTagContainer Tags;
    
    // 태그 조작
    FORCEINLINE void AddTag(FGameplayTag Tag)
    {
        Tags.AddTag(Tag);
    }
    
    FORCEINLINE void RemoveTag(FGameplayTag Tag)
    {
        Tags.RemoveTag(Tag);
    }
    
    FORCEINLINE bool HasTag(FGameplayTag Tag) const
    {
        return Tags.HasTag(Tag);
    }
    
    FORCEINLINE bool HasAny(const FGameplayTagContainer& Other) const
    {
        return Tags.HasAny(Other);
    }
    
    FORCEINLINE bool HasAll(const FGameplayTagContainer& Other) const
    {
        return Tags.HasAll(Other);
    }
    
    // 부모 태그 포함 검사
    FORCEINLINE bool HasTagExact(FGameplayTag Tag) const
    {
        return Tags.HasTagExact(Tag);
    }
    
    void Clear() { Tags.Reset(); }
    int32 Num() const { return Tags.Num(); }
    
    const FGameplayTagContainer& GetTags() const { return Tags; }
};

// ============================================================================
// FHktEntityState - 엔티티(Unit) 상태
// 
// 속성 집합 + 태그 집합
// ============================================================================

struct FHktEntityState
{
    int32 EntityID = INDEX_NONE;
    int32 Generation = 0;
    FGameplayTag EntityType;        // 엔티티 타입 태그
    
    FHktAttributeMap Attributes;    // 속성 집합
    FHktTagSet StatusTags;          // 상태 태그 (Burning, Stunned, etc.)
    FHktTagSet BuffTags;            // 버프 태그
    FHktTagSet EquipmentTags;       // 장비 태그
    
    // Transform (자주 접근하므로 별도 저장)
    int32 PosX = 0, PosY = 0, PosZ = 0;     // Position (Fixed)
    int32 VelX = 0, VelY = 0, VelZ = 0;     // Velocity (Fixed)
    int32 TargetX = 0, TargetY = 0, TargetZ = 0; // Target Position (Fixed)
    int32 RotationYaw = 0;                   // Rotation (Fixed, degrees * 100)
    
    // 상태 플래그 (빠른 접근용)
    uint32 Flags = 0;
    
    static constexpr uint32 FLAG_ACTIVE   = 1 << 0;
    static constexpr uint32 FLAG_ALIVE    = 1 << 1;
    static constexpr uint32 FLAG_MOVING   = 1 << 2;
    static constexpr uint32 FLAG_CASTING  = 1 << 3;
    
    bool IsValid() const { return EntityID != INDEX_NONE && (Flags & FLAG_ACTIVE); }
    bool IsAlive() const { return (Flags & FLAG_ALIVE) != 0; }
    
    FVector GetPosition() const { return FVector(HktFixedToFloat(PosX), HktFixedToFloat(PosY), HktFixedToFloat(PosZ)); }
    void SetPosition(const FVector& V) { PosX = HktFloatToFixed(V.X); PosY = HktFloatToFixed(V.Y); PosZ = HktFloatToFixed(V.Z); }
    
    FVector GetVelocity() const { return FVector(HktFixedToFloat(VelX), HktFixedToFloat(VelY), HktFixedToFloat(VelZ)); }
    void SetVelocity(const FVector& V) { VelX = HktFloatToFixed(V.X); VelY = HktFloatToFixed(V.Y); VelZ = HktFloatToFixed(V.Z); }
    
    void Reset()
    {
        EntityID = INDEX_NONE;
        Generation = 0;
        EntityType = FGameplayTag();
        Attributes.Clear();
        StatusTags.Clear();
        BuffTags.Clear();
        EquipmentTags.Clear();
        PosX = PosY = PosZ = 0;
        VelX = VelY = VelZ = 0;
        TargetX = TargetY = TargetZ = 0;
        RotationYaw = 0;
        Flags = 0;
    }
};

// ============================================================================
// FHktPlayerState - 플레이어 상태
// 
// 여러 엔티티를 소유할 수 있는 플레이어 수준 상태
// ============================================================================

struct FHktPlayerState
{
    int32 PlayerID = INDEX_NONE;
    
    FHktAttributeMap Attributes;    // 플레이어 수준 속성 (Gold, Score, etc.)
    FHktTagSet PermissionTags;      // 권한 태그
    FHktTagSet AchievementTags;     // 업적 태그
    FHktTagSet UnlockTags;          // 해금 태그
    
    // 소유 엔티티 목록
    TArray<int32> OwnedEntityIDs;
    
    bool IsValid() const { return PlayerID != INDEX_NONE; }
    
    void Reset()
    {
        PlayerID = INDEX_NONE;
        Attributes.Clear();
        PermissionTags.Clear();
        AchievementTags.Clear();
        UnlockTags.Clear();
        OwnedEntityIDs.Empty();
    }
};

// ============================================================================
// FHktProcessState - VM 프로세스 상태
// 
// 현재 진행 중인 Flow의 상태
// ============================================================================

struct FHktProcessState
{
    int32 ProcessID = INDEX_NONE;       // VM Slot
    FGameplayTag FlowTag;               // 실행 중인 Flow 태그
    
    FHktAttributeMap LocalAttributes;   // 프로세스 로컬 속성
    FHktTagSet DoingTags;               // 현재 진행 중인 작업 태그
    FHktTagSet CompletedTags;           // 완료된 작업 태그
    FHktTagSet ContextTags;             // 컨텍스트 태그
    
    // 진행 상태
    int32 Progress = 0;                 // 진행도 (0-10000, percentage * 100)
    int32 StartTime = 0;                // 시작 시간 (Fixed)
    int32 Duration = 0;                 // 예상 소요 시간 (Fixed)
    
    bool IsValid() const { return ProcessID != INDEX_NONE; }
    
    void Reset()
    {
        ProcessID = INDEX_NONE;
        FlowTag = FGameplayTag();
        LocalAttributes.Clear();
        DoingTags.Clear();
        CompletedTags.Clear();
        ContextTags.Clear();
        Progress = 0;
        StartTime = 0;
        Duration = 0;
    }
};
