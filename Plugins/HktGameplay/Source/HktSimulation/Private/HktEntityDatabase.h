// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HktAttributeSet.h"

// Forward declarations
class AActor;

/**
 * Handles & Databases (Pure Data Containers)
 * 엔티티와 플레이어를 식별하는 핸들 및 데이터베이스
 * 
 * 외부 연결(Component 등)은 HktIntent 모듈에서 담당합니다.
 * 이 모듈은 순수 시뮬레이션 데이터만 관리합니다.
 */

struct FUnitHandle
{
	int32 Index = INDEX_NONE;
	int32 Generation = 0;

	FUnitHandle() = default;
	FUnitHandle(int32 InIndex, int32 InGen) : Index(InIndex), Generation(InGen) {}

	FORCEINLINE bool IsValid() const { return Index != INDEX_NONE; }
	
	FORCEINLINE bool operator==(const FUnitHandle& Other) const
	{
		return Index == Other.Index && Generation == Other.Generation;
	}

	FORCEINLINE bool operator!=(const FUnitHandle& Other) const
	{
		return !(*this == Other);
	}

	friend FORCEINLINE uint32 GetTypeHash(const FUnitHandle& Handle)
	{
		return HashCombine(GetTypeHash(Handle.Index), GetTypeHash(Handle.Generation));
	}
};

struct FPlayerHandle
{
	int32 Index = INDEX_NONE;
	// 플레이어는 보통 세대(Generation) 구분이 덜 필요하지만, 재접속 등을 고려해 확장 가능
	
	FPlayerHandle() = default;
	FPlayerHandle(int32 InIndex) : Index(InIndex) {}
	
	FORCEINLINE bool IsValid() const { return Index != INDEX_NONE; }
	
	FORCEINLINE bool operator==(const FPlayerHandle& Other) const 
	{ 
		return Index == Other.Index; 
	}

	FORCEINLINE bool operator!=(const FPlayerHandle& Other) const
	{
		return Index != Other.Index;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FPlayerHandle& Handle)
	{
		return GetTypeHash(Handle.Index);
	}
};

/**
 * Dirty flags for player attributes
 * Bitmask for cache-friendly change tracking
 */
enum class EPlayerDirtyFlag : uint8
{
	None = 0,
	Attributes = 1 << 0,      // Any attribute changed
	Health = 1 << 1,          // Health specifically changed
	Resources = 1 << 2,       // Mana/resources changed
	Stats = 1 << 3,           // Attack/Defense changed
	All = 0xFF
};
ENUM_CLASS_FLAGS(EPlayerDirtyFlag);

/**
 * [Player Database]
 * 플레이어 단위의 전역 데이터 (팀 자원, 업그레이드 상태 등)
 * 
 * Cache-friendly SoA design with dirty flag tracking
 * Component 연결은 HktIntent에서 Provider 인터페이스를 통해 처리합니다.
 */
struct FHktPlayerDatabase
{
	// --- Logic Data (SoA for cache efficiency) ---
	TArray<FHktAttributeSet> Attributes;
	TArray<bool> IsActive;
	
	// Dirty flags for change tracking (Provider에서 조회 시 사용)
	// Bitmask array for efficient batch synchronization
	TArray<uint8> DirtyFlags;
	
	// 필요 시 팀 정보, 닉네임 인덱스 등 추가
	
	/**
	 * Add a new player to the database
	 * @return Index of the new player
	 */
	int32 AddPlayer()
	{
		int32 Idx = Attributes.Add(FHktAttributeSet());
		IsActive.Add(true);
		DirtyFlags.Add(static_cast<uint8>(EPlayerDirtyFlag::All)); // 신규 플레이어는 전체 동기화 필요
		return Idx;
	}
	
	/**
	 * Remove a player from the database
	 */
	void RemovePlayer(int32 PlayerIndex)
	{
		if (IsActive.IsValidIndex(PlayerIndex))
		{
			IsActive[PlayerIndex] = false;
			DirtyFlags[PlayerIndex] = static_cast<uint8>(EPlayerDirtyFlag::None);
		}
	}
	
	/**
	 * Mark player as dirty for synchronization
	 * @param PlayerIndex - Index of the player
	 * @param Flags - Which attributes changed
	 */
	FORCEINLINE void MarkDirty(int32 PlayerIndex, EPlayerDirtyFlag Flags = EPlayerDirtyFlag::Attributes)
	{
		if (DirtyFlags.IsValidIndex(PlayerIndex))
		{
			DirtyFlags[PlayerIndex] |= static_cast<uint8>(Flags);
		}
	}
	
	/**
	 * Clear dirty flag for a player
	 */
	FORCEINLINE void ClearDirty(int32 PlayerIndex)
	{
		if (DirtyFlags.IsValidIndex(PlayerIndex))
		{
			DirtyFlags[PlayerIndex] = static_cast<uint8>(EPlayerDirtyFlag::None);
		}
	}
	
	/**
	 * Check if player has dirty attributes
	 */
	FORCEINLINE bool IsDirty(int32 PlayerIndex) const
	{
		return DirtyFlags.IsValidIndex(PlayerIndex) && DirtyFlags[PlayerIndex] != 0;
	}
};

/**
 * [Entity Database]
 * 유닛 단위 데이터. 로직은 Manager로 위임하고 데이터 저장에만 집중합니다.
 */
struct FHktEntityDatabase
{
	// --- Logic Data ---
	TArray<FHktAttributeSet> Attributes;
	TArray<FVector> Locations;
	TArray<FRotator> Rotations;
	
	// [New] Player Link: 이 유닛의 소유주
	TArray<int32> OwnerPlayerIndices;

	// --- Meta Data ---
	TArray<int32> Generations;
	TArray<bool> IsActive;
	
	// [Optimization] External IDs stored directly in database
	// Internal Index -> External ID mapping (O(1) reverse lookup)
	TArray<int32> ExternalIds;

	// --- Visual Link ---
	TArray<TWeakObjectPtr<AActor>> VisualActors;
};
