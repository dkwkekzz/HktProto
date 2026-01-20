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
        InArraySerializer.OwnerSubsystem->AddEvent(Event);
    }
}

void FHktEventItem::PostReplicatedChange(const FHktEventContainer& InArraySerializer)
{
    // Lockstep 방식에서는 Update 없음 (추가만)
    // 혹시 발생하면 로그
    UE_LOG(LogTemp, Warning, TEXT("[FHktEventItem] PostReplicatedChange called - unexpected in Lockstep mode"));
}

void FHktEventItem::PreReplicatedRemove(const FHktEventContainer& InArraySerializer)
{
    // Lockstep 방식에서는 Commit 시 서버에서만 제거
    // 클라이언트에서 제거 복제되면 로그만 남김
    UE_LOG(LogTemp, Verbose, TEXT("[FHktEventItem] PreReplicatedRemove called for EventId: %d"), Event.EventId);
}

FHktEventItem& FHktEventContainer::AddOrUpdateItem(const FHktEventItem& Item)
{
    // Lockstep 방식: 추가만 허용, Update는 불필요
    FHktEventItem& NewItem = Items.Add_GetRef(Item);
    MarkItemDirty(NewItem);
    
    // 서버에서 직접 Subsystem에 동기화
    if (OwnerSubsystem)
    {
        OwnerSubsystem->AddEvent(NewItem.Event);
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

void UHktIntentEventComponent::RemoveProcessedEvents(int32 LastProcessedEventId)
{
    if (GetOwnerRole() != ROLE_Authority)
    {
        return;
    }
    
    const int32 RemovedCount = EventBuffer.Items.RemoveAll([LastProcessedEventId](const FHktEventItem& Item) {
        return Item.Event.EventId <= LastProcessedEventId;
    });
    
    if (RemovedCount > 0)
    {
        EventBuffer.MarkArrayDirty();
        UE_LOG(LogTemp, Verbose, TEXT("[HktIntentEventComponent] Removed %d processed events (EventId <= %d)"), 
            RemovedCount, LastProcessedEventId);
    }
}
