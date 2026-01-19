// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Data-Oriented Attribute System
 * 유닛과 플레이어가 공통으로 사용하는 속성 시스템
 */

enum class EHktAttribute : uint8
{
	Health = 0,
	MaxHealth,
	Mana,
	MaxMana,
	AttackPower,
	Defense,
	MoveSpeed,
	Count
};

/**
 * [POD Attribute Set]
 * 유닛과 플레이어가 공통으로 사용하는 속성 데이터
 */
struct FHktAttributeSet
{
	float Values[static_cast<uint8>(EHktAttribute::Count)];

	FHktAttributeSet()
	{
		FMemory::Memzero(Values, sizeof(Values));
		Set(EHktAttribute::MaxHealth, 100.0f);
		Set(EHktAttribute::Health, 100.0f);
		Set(EHktAttribute::MoveSpeed, 600.0f);
	}

	FORCEINLINE float Get(EHktAttribute Attr) const 
	{ 
		return Values[static_cast<uint8>(Attr)]; 
	}
	
	FORCEINLINE void Set(EHktAttribute Attr, float NewValue) 
	{ 
		Values[static_cast<uint8>(Attr)] = NewValue; 
	}
	
	void Modify(EHktAttribute Attr, float Delta)
	{
		Values[static_cast<uint8>(Attr)] += Delta;
		if (Attr == EHktAttribute::Health)
		{
			float Max = Get(EHktAttribute::MaxHealth);
			Values[static_cast<uint8>(Attr)] = FMath::Clamp(Values[static_cast<uint8>(Attr)], 0.0f, Max);
		}
	}
};
