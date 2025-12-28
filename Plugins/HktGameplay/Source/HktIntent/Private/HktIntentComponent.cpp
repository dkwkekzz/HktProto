#include "HktIntentComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "HktIntentGameState.h"
#include "HktActionDataAsset.h"
#include "HktIntentSubsystem.h"

// 인터페이스 헤더 포함 필요 (ResolveSubjects 반환 타입 확인용)
#include "HktInterfaces.h" 

static const int32 SERVER_INTENT_DELAY_FRAMES = 5;

// ============================================================================
// [FAS Item Logic]
// ============================================================================

void FHktIntentItem::PostReplicatedAdd(const FHktIntentContainer& InArraySerializer)
{
    NotifySubsystem(InArraySerializer, EIntentChangeType::Added);
}

void FHktIntentItem::PostReplicatedChange(const FHktIntentContainer& InArraySerializer)
{
    NotifySubsystem(InArraySerializer, EIntentChangeType::Updated);
}

void FHktIntentItem::PreReplicatedRemove(const FHktIntentContainer& InArraySerializer)
{
    NotifySubsystem(InArraySerializer, EIntentChangeType::Removed);
}

void FHktIntentItem::NotifySubsystem(const FHktIntentContainer& InArraySerializer, EIntentChangeType ChangeType) const
{
    if (InArraySerializer.OwnerComponent)
    {
        InArraySerializer.OwnerComponent->ReportIntentChangeToSubsystem(Event, ChangeType);
    }
}

FHktIntentItem& FHktIntentContainer::AddOrUpdateItem(const FHktIntentItem& Item)
{
    FHktIntentItem* ExistingItem = Items.FindByPredicate([&](const FHktIntentItem& Existing)
    {
        return Existing.Event.EventId == Item.Event.EventId;
    });

    if (ExistingItem)
    {
        *ExistingItem = Item;
        MarkItemDirty(*ExistingItem);
        if (OwnerComponent)
        {
            OwnerComponent->ReportIntentChangeToSubsystem(ExistingItem->Event, EIntentChangeType::Updated);
        }
        return *ExistingItem;
    }

    FHktIntentItem& NewItem = Items.Add_GetRef(Item);
    MarkItemDirty(NewItem);
    if (OwnerComponent)
    {
        OwnerComponent->ReportIntentChangeToSubsystem(NewItem.Event, EIntentChangeType::Added);
    }
    return NewItem;
}

// ============================================================================
// [Component Logic]
// ============================================================================

UHktIntentComponent::UHktIntentComponent()
{
    SetIsReplicatedByDefault(true);
    IntentBuffer.OwnerComponent = this;
    LocalIntentSequence = 0;
}

void UHktIntentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(UHktIntentComponent, IntentBuffer, COND_None);
}

void UHktIntentComponent::BeginPlay()
{
    Super::BeginPlay();
    if (UWorld* World = GetWorld())
    {
        CachedSubsystem = World->GetSubsystem<UHktIntentSubsystem>();
    }
}

void UHktIntentComponent::SubmitIntent(const TScriptInterface<IHktSubjectContext> Subject, 
                                       const TScriptInterface<IHktCommandContext> Command, 
                                       const TScriptInterface<IHktTargetContext> Target)
{
    if (!Subject || !Command) 
	{
		UE_LOG(LogTemp, Error, TEXT("Subject, Command, or Target is not valid"));
		return;
	}

    if (!Subject->IsValid() || !Command->IsValid()) 
	{
		UE_LOG(LogTemp, Error, TEXT("Subject, Command, or Target is not valid"));
		return;
	}

    UHktActionDataAsset* Action = Command->ResolveAction();
    if (!Action) 
	{
		UE_LOG(LogTemp, Error, TEXT("Action is not valid"));
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
	NewEvent.Subjects = Subject->ResolveSubjects();
	NewEvent.EventTag = Action->EventTag;
	if (Target)
	{
		NewEvent.Target = Target->GetTargetUnit();
		NewEvent.Location = Target->GetTargetLocation();
	}

	Server_ReceiveEvent(NewEvent);
}

void UHktIntentComponent::ReportIntentChangeToSubsystem(const FHktIntentEvent& Event, EIntentChangeType ChangeType)
{
    UHktIntentSubsystem* Subsystem = CachedSubsystem.Get();
    if (!Subsystem)
    {
        if (UWorld* World = GetWorld())
        {
            Subsystem = World->GetSubsystem<UHktIntentSubsystem>();
            CachedSubsystem = Subsystem;
        }
    }

    if (Subsystem)
    {
        // Event 내부에 Subjects(핸들)가 포함되어 있으므로, Owner Actor를 넘길 필요 없이 Event만 전달
        Subsystem->ProcessIntentChange(Event, ChangeType);
    }
}

void UHktIntentComponent::Server_ReceiveEvent_Implementation(FHktIntentEvent PendingEvent)
{
    int32 CurrentServerFrame = 0;
    if (const AHktIntentGameState* GS = GetWorld()->GetGameState<AHktIntentGameState>())
    {
        CurrentServerFrame = GS->GetCurrentFrame();
    }

    PendingEvent.FrameNumber = CurrentServerFrame + SERVER_INTENT_DELAY_FRAMES;
    IntentBuffer.AddOrUpdateItem(FHktIntentItem(PendingEvent));
}

bool UHktIntentComponent::Server_ReceiveEvent_Validate(FHktIntentEvent PendingEvent)
{
    return true; 
}