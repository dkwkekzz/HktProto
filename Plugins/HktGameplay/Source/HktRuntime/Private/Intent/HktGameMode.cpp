#include "HktGameMode.h"
#include "HktPlayerController.h"
#include "HktIntentEventComponent.h"
#include "HktMasterStashComponent.h"

AHktGameMode::AHktGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    // MasterStash 컴포넌트 생성
    MasterStash = CreateDefaultSubobject<UHktMasterStashComponent>(TEXT("MasterStash"));
}

void AHktGameMode::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("HktGameMode: Started. MasterStash=%s"), 
        MasterStash ? TEXT("Valid") : TEXT("Invalid"));
}

void AHktGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // 틱마다 배치 처리
    ProcessIntentBatch();

    CurrentServerTick++;
}

void AHktGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (AHktPlayerController* HktPC = Cast<AHktPlayerController>(NewPlayer))
    {
        RegisteredControllers.Add(HktPC);
        
        // IntentEventComponent 델리게이트 바인딩
        if (UHktIntentEventComponent* IntentComp = HktPC->GetIntentEventComponent())
        {
            IntentComp->OnServerReceivedIntent.AddDynamic(
                this, &AHktGameMode::OnServerReceivedIntent);
        }

        UE_LOG(LogTemp, Log, TEXT("HktGameMode: Player logged in - %s"), *HktPC->GetName());
    }
}

void AHktGameMode::Logout(AController* Exiting)
{
    if (AHktPlayerController* HktPC = Cast<AHktPlayerController>(Exiting))
    {
        // 델리게이트 언바인딩
        if (UHktIntentEventComponent* IntentComp = HktPC->GetIntentEventComponent())
        {
            IntentComp->OnServerReceivedIntent.RemoveDynamic(
                this, &AHktGameMode::OnServerReceivedIntent);
        }

        RegisteredControllers.RemoveAll([HktPC](const TWeakObjectPtr<AHktPlayerController>& WeakPC)
        {
            return !WeakPC.IsValid() || WeakPC.Get() == HktPC;
        });

        UE_LOG(LogTemp, Log, TEXT("HktGameMode: Player logged out - %s"), *HktPC->GetName());
    }

    Super::Logout(Exiting);
}

void AHktGameMode::OnServerReceivedIntent(const FHktIntentEvent& Event)
{
    // Intent ID 할당 및 서버 틱 기록
    FHktIntentEvent ProcessedEvent = Event;
    ProcessedEvent.EventId = NextIntentId++;
    ProcessedEvent.FrameNumber = CurrentServerTick;

    UE_LOG(LogTemp, Verbose, TEXT("HktGameMode: Received Intent #%d [%s]"),
        ProcessedEvent.EventId,
        *ProcessedEvent.EventTag.ToString());

    PendingIntents.Add(ProcessedEvent);
}

void AHktGameMode::ProcessIntentBatch()
{
    if (PendingIntents.IsEmpty())
    {
        return;
    }

    UE_LOG(LogTemp, Verbose, TEXT("HktGameMode: Processing batch with %d intents at tick %d"),
        PendingIntents.Num(), CurrentServerTick);

    // 무효한 컨트롤러 정리
    RegisteredControllers.RemoveAll([](const TWeakObjectPtr<AHktPlayerController>& WeakPC)
    {
        return !WeakPC.IsValid();
    });

    // 각 클라이언트에게 배치 전송
    for (const auto& WeakPC : RegisteredControllers)
    {
        AHktPlayerController* PC = WeakPC.Get();
        if (!PC)
        {
            continue;
        }

        UHktIntentEventComponent* IntentComp = PC->GetIntentEventComponent();
        if (!IntentComp)
        {
            continue;
        }

        // 이 클라이언트용 배치 생성
        FHktIntentEventBatch Batch;
        Batch.FrameNumber = CurrentServerTick;

        for (const FHktIntentEvent& Event : PendingIntents)
        {
            // Relevancy 체크
            if (!IsEventRelevantToClient(PC, Event))
            {
                continue;
            }

            // 이 클라이언트용으로 이벤트 준비 (스냅샷 첨부)
            FHktIntentEvent EventForClient = PrepareEventForClient(PC, Event);
            Batch.Events.Add(EventForClient);
        }

        // 배치가 비어있지 않으면 전송
        if (!Batch.IsEmpty())
        {
            IntentComp->SendBatchToClient(Batch);
            
            UE_LOG(LogTemp, Verbose, TEXT("HktGameMode: Sent batch with %d events to %s"),
                Batch.Num(), *PC->GetName());
        }
    }

    // 서버에서도 실행 (TODO: VMProcessor 연동)
    for (const FHktIntentEvent& Event : PendingIntents)
    {
        VMProcessor.QueueIntentEvent(Event);
    }

    // 배치 처리 완료, 클리어
    PendingIntents.Empty();
}

FHktIntentEvent AHktGameMode::PrepareEventForClient(
    AHktPlayerController* ClientPC, 
    const FHktIntentEvent& OriginalEvent)
{
    FHktIntentEvent EventForClient = OriginalEvent;
    EventForClient.AttachedSnapshots.Empty(); // 스냅샷은 새로 채움

    UHktIntentEventComponent* IntentComp = ClientPC->GetIntentEventComponent();
    if (!IntentComp || !MasterStash)
    {
        return EventForClient;
    }

    // Source 엔티티 스냅샷 필요 여부 체크
    if (OriginalEvent.SubjectEntityId != InvalidEntityId)
    {
        if (!IntentComp->KnowsEntity(OriginalEvent.SubjectEntityId))
        {
            // 스냅샷 생성 및 첨부
            FHktEntitySnapshot Snapshot = MasterStash->CreateEntitySnapshot(OriginalEvent.SubjectEntityId);
            if (Snapshot.IsValid())
            {
                EventForClient.AttachedSnapshots.Add(Snapshot);
                IntentComp->MarkEntityKnown(OriginalEvent.SubjectEntityId);
                
                UE_LOG(LogTemp, Verbose, TEXT("  Attached snapshot for Source Entity %d"),
                    OriginalEvent.SubjectEntityId);
            }
        }
    }

    // Target 엔티티 스냅샷 필요 여부 체크
    if (OriginalEvent.TargetEntityId != InvalidEntityId)
    {
        if (!IntentComp->KnowsEntity(OriginalEvent.TargetEntityId))
        {
            // 스냅샷 생성 및 첨부
            FHktEntitySnapshot Snapshot = MasterStash->CreateEntitySnapshot(OriginalEvent.TargetEntityId);
            if (Snapshot.IsValid())
            {
                EventForClient.AttachedSnapshots.Add(Snapshot);
                IntentComp->MarkEntityKnown(OriginalEvent.TargetEntityId);
                
                UE_LOG(LogTemp, Verbose, TEXT("  Attached snapshot for Target Entity %d"),
                    OriginalEvent.TargetEntityId);
            }
        }
    }

    return EventForClient;
}

bool AHktGameMode::IsEntityRelevantToClient(AHktPlayerController* ClientPC, FHktEntityId EntityId) const
{
    if (!ClientPC || EntityId == InvalidEntityId)
    {
        return false;
    }

    // TODO: 실제 Relevancy 로직 구현
    // - 거리 기반
    // - 시야 기반
    // - 팀 기반 등

    // 기본적으로 모든 엔티티가 관련있다고 가정
    return true;
}

bool AHktGameMode::IsEventRelevantToClient(AHktPlayerController* ClientPC, const FHktIntentEvent& Event) const
{
    if (!ClientPC)
    {
        return false;
    }

    // Source나 Target 중 하나라도 관련있으면 이벤트도 관련있음
    bool bSourceRelevant = Event.SubjectEntityId == InvalidEntityId || 
                           IsEntityRelevantToClient(ClientPC, Event.SubjectEntityId);
    bool bTargetRelevant = Event.TargetEntityId == InvalidEntityId || 
                           IsEntityRelevantToClient(ClientPC, Event.TargetEntityId);

    return bSourceRelevant || bTargetRelevant;
}