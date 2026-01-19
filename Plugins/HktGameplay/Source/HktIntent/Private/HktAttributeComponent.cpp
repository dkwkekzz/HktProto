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
		// 기존 항목 업데이트
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

void FHktAttributeContainer::ApplySnapshot(const FHktPlayerAttributeSnapshot& Snapshot)
{
	// ChangedAttributes가 있으면 그것만 적용 (최적화)
	if (Snapshot.ChangedAttributes.Num() > 0)
	{
		for (const FHktAttributeEntry& Entry : Snapshot.ChangedAttributes)
		{
			SetAttribute(Entry.AttributeType, Entry.Value);
		}
	}
	// ChangedAttributes가 비어있으면 AllAttributes 전체 적용
	else if (Snapshot.AllAttributes.Num() > 0)
	{
		for (int32 i = 0; i < Snapshot.AllAttributes.Num() && i < static_cast<int32>(EHktAttributeType::Count); ++i)
		{
			SetAttribute(static_cast<EHktAttributeType>(i), Snapshot.AllAttributes[i]);
		}
	}
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

void UHktAttributeComponent::ApplyAttributeSnapshot(const FHktPlayerAttributeSnapshot& Snapshot)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	AttributeContainer.ApplySnapshot(Snapshot);
}

void UHktAttributeComponent::NotifyAttributeChanged(EHktAttributeType Type, float NewValue)
{
	// 델리게이트 브로드캐스트 (UI 업데이트 등)
	OnAttributeChanged.Broadcast(Type, NewValue);
}
