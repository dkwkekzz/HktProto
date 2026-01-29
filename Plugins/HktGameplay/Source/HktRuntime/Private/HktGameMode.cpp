#include "HktGameMode.h"
#include "HktPlayerController.h"
#include "HktMasterStashComponent.h"
#include "HktGridRelevancyComponent.h"
#include "HktVMProcessorComponent.h"
#include "Async/ParallelFor.h"

AHktGameMode::AHktGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    MasterStash = CreateDefaultSubobject<UHktMasterStashComponent>(TEXT("MasterStash"));
    GridRelevancy = CreateDefaultSubobject<UHktGridRelevancyComponent>(TEXT("GridRelevancy"));
    VMProcessor = CreateDefaultSubobject<UHktVMProcessorComponent>(TEXT("VMProcessor"));
}

void AHktGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // VMProcessor를 MasterStash와 연결
    if (VMProcessor && MasterStash)
    {
        VMProcessor->InitializeWithMasterStash(MasterStash);
    }
    
    UE_LOG(LogTemp, Log, TEXT("HktGameMode: Initialized"));
}

void AHktGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (GridRelevancy)
    {
        GridRelevancy->UpdateRelevancy(DeltaSeconds);
    }

    ProcessFrame();
    FrameNumber++;
}

void AHktGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (AHktPlayerController* HktPC = Cast<AHktPlayerController>(NewPlayer))
    {
        if (GridRelevancy)
        {
            GridRelevancy->RegisterClient(HktPC);
        }
        UE_LOG(LogTemp, Log, TEXT("HktGameMode: Player logged in - %s"), *HktPC->GetName());
    }
}

void AHktGameMode::Logout(AController* Exiting)
{
    if (AHktPlayerController* HktPC = Cast<AHktPlayerController>(Exiting))
    {
        if (GridRelevancy)
        {
            GridRelevancy->UnregisterClient(HktPC);
        }
    }
    Super::Logout(Exiting);
}

void AHktGameMode::PushIntent(const FHktIntentEvent& Event)
{
    FScopeLock Lock(&IntentLock);
    CollectedIntents.Add(Event);
}

void AHktGameMode::ProcessFrame()
{
    if (!GridRelevancy || !MasterStash)
    {
        return;
    }

    const TArray<AHktPlayerController*>& AllClients = GridRelevancy->GetAllClients();
    
    // 1. Intent 가져오기 (락 최소화)
    {
        FScopeLock Lock(&IntentLock);
        if (CollectedIntents.IsEmpty() && AllClients.IsEmpty())
        {
            return;
        }
        FrameIntents = MoveTemp(CollectedIntents);
        CollectedIntents.Reset();
    }

    // 2. 이벤트별 셀 정보 미리 계산 (메인 스레드)
    const int32 NumEvents = FrameIntents.Num();
    EventCellCache.SetNum(NumEvents);
    
    for (int32 i = 0; i < NumEvents; ++i)
    {
        const FHktIntentEvent& Event = FrameIntents[i];
        FEventCellInfo& Info = EventCellCache[i];
        
        Info.bIsGlobal = Event.bIsGlobal;
        
        if (Event.bIsGlobal)
        {
            Info.bHasValidLocation = false;
        }
        else
        {
            FVector SourceLocation;
            Info.bHasValidLocation = MasterStash->TryGetPosition(Event.SourceEntity, SourceLocation);
            if (Info.bHasValidLocation)
            {
                Info.Cell = GridRelevancy->LocationToCell(SourceLocation);
            }
        }
    }

    // 3. 클라이언트별 병렬 처리
    //    - 각 클라이언트는 독립적으로 자신의 배치 생성
    //    - 읽기 전용 데이터: FrameIntents, EventCellCache, GridRelevancy
    //    - 쓰기 데이터: 각 PC의 Relevancy, 각 PC의 Batch (독립적)
    
    const int32 NumClients = AllClients.Num();
    TArray<FHktFrameBatch> Batches;
    Batches.SetNum(NumClients);

    ParallelFor(NumClients, [&](int32 ClientIndex)
    {
        AHktPlayerController* PC = AllClients[ClientIndex];
        FHktFrameBatch& Batch = Batches[ClientIndex];
        Batch.FrameNumber = FrameNumber;

        FHktClientRelevancy& Relevancy = PC->GetRelevancy();
        Relevancy.BeginFrame();

        // 이 클라이언트에게 관련된 이벤트 필터링
        for (int32 EventIndex = 0; EventIndex < NumEvents; ++EventIndex)
        {
            const FHktIntentEvent& Event = FrameIntents[EventIndex];
            const FEventCellInfo& Info = EventCellCache[EventIndex];

            bool bRelevant = false;
            
            if (Info.bIsGlobal)
            {
                bRelevant = true;
            }
            else if (Info.bHasValidLocation)
            {
                bRelevant = GridRelevancy->IsClientInterestedInCell(PC, Info.Cell);
            }
            else
            {
                // 위치 없는 이벤트는 글로벌 처리
                bRelevant = true;
            }

            if (bRelevant)
            {
                Batch.Events.Add(Event);

                // 관련 엔티티 Relevancy 진입
                if (Event.SourceEntity != InvalidEntityId)
                {
                    Relevancy.EnterRelevancy(Event.SourceEntity);
                }
                if (Event.TargetEntity != InvalidEntityId)
                {
                    Relevancy.EnterRelevancy(Event.TargetEntity);
                }
            }
        }

        // 새로 진입한 엔티티 스냅샷 추가
        for (FHktEntityId EntityId : Relevancy.EnteredEntities)
        {
            FHktEntitySnapshot Snapshot = MasterStash->CreateEntitySnapshot(EntityId);
            if (Snapshot.IsValid())
            {
                Batch.Snapshots.Add(Snapshot);
            }
        }

        // 이탈한 엔티티 제거 목록 추가
        for (FHktEntityId EntityId : Relevancy.ExitedEntities)
        {
            Batch.RemovedEntities.Add(EntityId);
        }
    });

    // 4. 배치 전송 (메인 스레드 - RPC는 메인에서)
    for (int32 i = 0; i < NumClients; ++i)
    {
        if (!Batches[i].IsEmpty())
        {
            AllClients[i]->SendBatchToOwningClient(Batches[i]);
        }
    }

    // 5. 서버 VMProcessor 실행
    if (VMProcessor && VMProcessor->IsInitialized())
    {
        // 모든 이벤트를 VMProcessor에 큐잉
        VMProcessor->QueueIntentEvents(FrameIntents);
        
        // 프레임 처리 (Build → Execute → Cleanup)
        VMProcessor->ProcessFrame(FrameNumber, GetWorld()->GetDeltaSeconds());
    }
}