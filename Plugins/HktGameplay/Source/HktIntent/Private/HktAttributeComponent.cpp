// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktAttributeComponent.h"
#include "Net/UnrealNetwork.h"

// ============================================================================
// FHktAttributeItem - FFastArraySerializerItem Callbacks
// ============================================================================

void FHktAttributeItem::PostReplicatedAdd(const FHktAttributeContainer& InArraySerializer)
{
	if (InArraySerializer.OwnerComponent)
	{
		InArraySerializer.OwnerComponent->NotifyAttributeChanged(AttributeType, Value);
	}
}

void FHktAttributeItem::PostReplicatedChange(const FHktAttributeContainer& InArraySerializer)
{
	if (InArraySerializer.OwnerComponent)
	{
		InArraySerializer.OwnerComponent->NotifyAttributeChanged(AttributeType, Value);
	}
}

void FHktAttributeItem::PreReplicatedRemove(const FHktAttributeContainer& InArraySerializer)
{
	// 속성 제거는 일반적으로 발생하지 않음
}

// ============================================================================
// FHktAttributeContainer
// ============================================================================

void FHktAttributeContainer::SetAttribute(EHktAttributeType Type, float NewValue)
{
	const int32 ItemIdx = FindItemIndex(Type);
	
	if (ItemIdx != INDEX_NONE)
	{
		// 기존 항목 업데이트 (값이 변경된 경우에만)
		if (!FMath::IsNearlyEqual(Items[ItemIdx].Value, NewValue))
		{
			Items[ItemIdx].Value = NewValue;
			MarkItemDirty(Items[ItemIdx]);
		}
	}
	else
	{
		// 새 항목 추가
		FHktAttributeItem NewItem(Type, NewValue);
		Items.Add(NewItem);
		MarkArrayDirty();
	}
}

void FHktAttributeContainer::SetAllAttributes(const TArray<float>& Values)
{
	for (int32 i = 0; i < Values.Num() && i < static_cast<int32>(EHktAttributeType::Count); ++i)
	{
		SetAttribute(static_cast<EHktAttributeType>(i), Values[i]);
	}
}

float FHktAttributeContainer::GetAttribute(EHktAttributeType Type) const
{
	const int32 ItemIdx = FindItemIndex(Type);
	return (ItemIdx != INDEX_NONE) ? Items[ItemIdx].Value : 0.0f;
}

void FHktAttributeContainer::InitializeDefaults()
{
	Items.Empty();
	
	// 기본 속성 초기화
	Items.Add(FHktAttributeItem(EHktAttributeType::Health, 100.0f));
	Items.Add(FHktAttributeItem(EHktAttributeType::MaxHealth, 100.0f));
	Items.Add(FHktAttributeItem(EHktAttributeType::Mana, 0.0f));
	Items.Add(FHktAttributeItem(EHktAttributeType::MaxMana, 100.0f));
	Items.Add(FHktAttributeItem(EHktAttributeType::AttackPower, 10.0f));
	Items.Add(FHktAttributeItem(EHktAttributeType::Defense, 5.0f));
	Items.Add(FHktAttributeItem(EHktAttributeType::MoveSpeed, 600.0f));
	
	MarkArrayDirty();
}

int32 FHktAttributeContainer::FindItemIndex(EHktAttributeType Type) const
{
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (Items[i].AttributeType == Type)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

// ============================================================================
// UHktAttributeComponent
// ============================================================================

UHktAttributeComponent::UHktAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UHktAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	// 컨테이너에 소유자 설정 (콜백에서 사용)
	AttributeContainer.OwnerComponent = this;

	// 서버에서만 기본값 초기화
	if (GetOwnerRole() == ROLE_Authority)
	{
		AttributeContainer.InitializeDefaults();
	}
}

void UHktAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHktAttributeComponent, PlayerHandle);
	DOREPLIFETIME(UHktAttributeComponent, AttributeContainer);
}

void UHktAttributeComponent::SetPlayerHandle(const FHktPlayerHandle& InHandle)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		PlayerHandle = InHandle;
	}
}

void UHktAttributeComponent::SetAttribute(EHktAttributeType Type, float Value)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		AttributeContainer.SetAttribute(Type, Value);
	}
}

void UHktAttributeComponent::SetAllAttributes(const TArray<float>& Values)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		AttributeContainer.SetAllAttributes(Values);
	}
}

void UHktAttributeComponent::NotifyAttributeChanged(EHktAttributeType Type, float NewValue)
{
	// 델리게이트 브로드캐스트 (UI 업데이트 등)
	OnAttributeChanged.Broadcast(Type, NewValue);
}
