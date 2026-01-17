// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HktAttributeSet.h"

/**
 * Handles & Databases (Pure Data Containers)
 * 엔티티와 플레이어를 식별하는 핸들 및 데이터베이스
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
 * [Player Database]
 * 플레이어 단위의 전역 데이터 (팀 자원, 업그레이드 상태 등)
 */
struct FHktPlayerDatabase
{
	// SoA Arrays
	TArray<FHktAttributeSet> Attributes;
	TArray<bool> IsActive;
	
	// 필요 시 팀 정보, 닉네임 인덱스 등 추가
	
	int32 AddPlayer()
	{
		int32 Idx = Attributes.Add(FHktAttributeSet());
		IsActive.Add(true);
		return Idx;
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

	// --- Visual Link ---
	TArray<TWeakObjectPtr<AActor>> VisualActors;
};
