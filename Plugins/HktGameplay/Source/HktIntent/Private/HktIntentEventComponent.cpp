#include "HktIntentEventComponent.h"
#include "HktIntentBuilderComponent.h"
#include "HktIntentGameMode.h"
#include "HktIntentSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

// ============================================================================
// [FAS Item Logic]
// ============================================================================

void FHktEventItem::PostReplicatedAdd(const FHktEventContainer& InArraySerializer)
{
    if (InArraySerializer.OwnerSubsystem)
    {
        InArraySerializer.OwnerSubsystem->CreateOrGetChannel(InArraySerializer.ChannelId)->AddEvent(Event);
    }
}

void FHktEventItem::PostReplicatedChange(const FHktEventContainer& InArraySerializer)
{
    if (InArraySerializer.OwnerSubsystem)
    {
        InArraySerializer.OwnerSubsystem->CreateOrGetChannel(InArraySerializer.ChannelId)->UpdateEvent(Event);
    }
}

void FHktEventItem::PreReplicatedRemove(const FHktEventContainer& InArraySerializer)
{
    if (InArraySerializer.OwnerSubsystem)
    {
        InArraySerializer.OwnerSubsystem->CreateOrGetChannel(InArraySerializer.ChannelId)->RemoveEvent(Event);
    }
}

FHktEventItem& FHktEventContainer::AddOrUpdateItem(const FHktEventItem& Item)
{
    FHktEventItem* ExistingItem = Items.FindByPredicate([&](const FHktEventItem& Existing)
    {
        return Existing.Event.EventId == Item.Event.EventId;
    });

    if (ExistingItem)
    {
        *ExistingItem = Item;
        MarkItemDirty(*ExistingItem);
        // 서버에서 직접 Subsystem에 동기화
        if (OwnerSubsystem)
        {
            OwnerSubsystem->CreateOrGetChannel(ChannelId)->UpdateEvent(ExistingItem->Event);
        }
        return *ExistingItem;
    }

    FHktEventItem& NewItem = Items.Add_GetRef(Item);
    MarkItemDirty(NewItem);
    // 서버에서 직접 Subsystem에 동기화
    if (OwnerSubsystem)
    {
        OwnerSubsystem->CreateOrGetChannel(ChannelId)->AddEvent(ExistingItem->Event);
    }
    return NewItem;
}

// ============================================================================
// [Component Logic]
// ============================================================================

UHktIntentEventComponent::UHktIntentEventComponent()
{
    SetIsReplicatedByDefault(true);
    LocalIntentSequence = 0;
}

void UHktIntentEventComponent::BeginPlay()
{
    Super::BeginPlay();

    // Subsystem 캐싱
    EventBuffer.OwnerSubsystem = UHktIntentSubsystem::Get(GetWorld());
	
    if (UWorld* World = GetWorld())
    {
        // ChannelId 설정 (GameMode에서 가져옴)
        if (AHktIntentGameMode* GameMode = World->GetAuthGameMode<AHktIntentGameMode>())
        {
            EventBuffer.ChannelId = GameMode->GetChannelId();
        }
    }
}

void UHktIntentEventComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(UHktIntentEventComponent, EventBuffer, COND_None);
}

void UHktIntentEventComponent::SubmitIntent(UHktIntentBuilderComponent* Builder)
{
    if (!Builder)
    {
        UE_LOG(LogTemp, Error, TEXT("IntentBuilder is null"));
        return;
    }

    if (!Builder->IsReadyToSubmit())
    {
        UE_LOG(LogTemp, Error, TEXT("IntentBuilder is not ready to submit"));
        return;
    }

    // 클라이언트(AutonomousProxy)인 경우에만 서버로 전송
    if (GetOwnerRole() != ROLE_AutonomousProxy)
    {
        UE_LOG(LogTemp, Error, TEXT("Not an autonomous proxy, role: %d"), GetOwnerRole());
        return;
    }

    // Builder에서 직접 데이터 접근
    FHktIntentEvent NewEvent;
    NewEvent.EventId = ++LocalIntentSequence;
    NewEvent.Subject = Builder->GetSubject();
    NewEvent.EventTag = Builder->GetEventTag();
    NewEvent.Target = Builder->GetTargetUnit();
    NewEvent.Location = Builder->GetTargetLocation();

    Server_ReceiveEvent(NewEvent);
}

void UHktIntentEventComponent::Server_ReceiveEvent_Implementation(FHktIntentEvent PendingEvent)
{
    // GameMode에서 FrameNumber와 ChannelId 설정
    if (UWorld* World = GetWorld())
    {
        if (AHktIntentGameMode* GameMode = World->GetAuthGameMode<AHktIntentGameMode>())
        {
            PendingEvent.FrameNumber = GameMode->GetServerFrame();
        }
    }
    
    // 버퍼에 추가 (자동으로 클라이언트에 복제됨)
    EventBuffer.AddOrUpdateItem(FHktEventItem(PendingEvent));
}

bool UHktIntentEventComponent::Server_ReceiveEvent_Validate(FHktIntentEvent PendingEvent)
{
    return true; 
}
