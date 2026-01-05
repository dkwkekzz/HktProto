#include "HktIntentComponent.h"
#include "HktIntentGameMode.h"
#include "HktIntentSubsystem.h"
#include "Objects/HktInputContexts.h" // Interface definitions
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

// ============================================================================
// [FAS Item Logic]
// ============================================================================

void FHktEventItem::PostReplicatedAdd(const FHktEventContainer& InArraySerializer)
{
    if (InArraySerializer.OwnerSubsystem)
    {
        InArraySerializer.OwnerSubsystem->AddIntentEvent(InArraySerializer.ChannelId, Event);
    }
}

void FHktEventItem::PostReplicatedChange(const FHktEventContainer& InArraySerializer)
{
    if (InArraySerializer.OwnerSubsystem)
    {
        InArraySerializer.OwnerSubsystem->UpdateIntentEvent(InArraySerializer.ChannelId, Event);
    }
}

void FHktEventItem::PreReplicatedRemove(const FHktEventContainer& InArraySerializer)
{
    if (InArraySerializer.OwnerSubsystem)
    {
        InArraySerializer.OwnerSubsystem->RemoveIntentEvent(InArraySerializer.ChannelId, Event);
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
            OwnerSubsystem->UpdateIntentEvent(ChannelId, ExistingItem->Event);
        }
        return *ExistingItem;
    }

    FHktEventItem& NewItem = Items.Add_GetRef(Item);
    MarkItemDirty(NewItem);
    // 서버에서 직접 Subsystem에 동기화
    if (OwnerSubsystem)
    {
        OwnerSubsystem->AddIntentEvent(ChannelId, NewItem.Event);
    }
    return NewItem;
}

// ============================================================================
// [Component Logic]
// ============================================================================

UHktIntentComponent::UHktIntentComponent()
{
    SetIsReplicatedByDefault(true);
    LocalIntentSequence = 0;
}

void UHktIntentComponent::BeginPlay()
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

void UHktIntentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(UHktIntentComponent, EventBuffer, COND_None);
}

void UHktIntentComponent::SubmitIntent(const TScriptInterface<IHktSubjectContext> Subject, 
                                       const TScriptInterface<IHktCommandContext> Command, 
                                       const TScriptInterface<IHktTargetContext> Target)
{
    if (!Subject || !Subject->IsValid()) 
	{
		UE_LOG(LogTemp, Error, TEXT("Subject is not valid"));
		return;
	}

    if (!Command || !Command->IsValid()) 
	{
		UE_LOG(LogTemp, Error, TEXT("Command is not valid"));
		return;
	}

    // 클라이언트(AutonomousProxy)인 경우에만 서버로 전송
    if (GetOwnerRole() != ROLE_AutonomousProxy)
    {
		UE_LOG(LogTemp, Error, TEXT("Not an autonomous proxy, role: %d"), GetOwnerRole());
		return;
	}
	
	FHktIntentEvent NewEvent;
	NewEvent.EventId = ++LocalIntentSequence;
	NewEvent.Subject = Subject->ResolvePrimarySubject();
	NewEvent.EventTag = Command->ResolveEventTag();
	if (Target)
	{
		NewEvent.Target = Target->GetTargetUnit();
		NewEvent.Location = Target->GetTargetLocation();
	}

	Server_ReceiveEvent(NewEvent);
}

void UHktIntentComponent::Server_ReceiveEvent_Implementation(FHktIntentEvent PendingEvent)
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

bool UHktIntentComponent::Server_ReceiveEvent_Validate(FHktIntentEvent PendingEvent)
{
    return true; 
}
