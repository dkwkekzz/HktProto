// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktPlayerController.h"
#include "Components/HktIntentBuilderComponent.h"
#include "Components/HktVisibleStashComponent.h"
#include "Components/HktVMProcessorComponent.h"
#include "HktGameMode.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

AHktPlayerController::AHktPlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
}

void AHktPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (DefaultMappingContext)
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
    
    // 클라이언트에서만 VisibleStashComponent 및 VMProcessorComponent 생성
    if (!HasAuthority())
    {
        IntentBuilderComponent = NewObject<UHktIntentBuilderComponent>(this, TEXT("IntentBuilderComponent"));
        IntentBuilderComponent->RegisterComponent();
        
        VisibleStashComponent = NewObject<UHktVisibleStashComponent>(this, TEXT("VisibleStashComponent"));
        VisibleStashComponent->RegisterComponent();
        
        VMProcessorComponent = NewObject<UHktVMProcessorComponent>(this, TEXT("VMProcessorComponent"));
        VMProcessorComponent->RegisterComponent();
        
        // VMProcessor를 VisibleStash와 연결
        if (VMProcessorComponent && VisibleStashComponent)
        {
            VMProcessorComponent->Initialize(VisibleStashComponent->GetStashInterface());
        }

        UE_LOG(LogTemp, Log, TEXT("HktPlayerController: Client initialized with VisibleStash and VMProcessor"));
    }
}

void AHktPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
    if (!EnhancedInput) 
        return;

    if (SubjectAction)
        EnhancedInput->BindAction(SubjectAction, ETriggerEvent::Triggered, this, &AHktPlayerController::OnSubjectAction);

    if (TargetAction)
        EnhancedInput->BindAction(TargetAction, ETriggerEvent::Triggered, this, &AHktPlayerController::OnTargetAction);

    if (ZoomAction)
        EnhancedInput->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &AHktPlayerController::OnZoom);

    for (int32 i = 0; i < SlotActions.Num(); ++i)
    {
        if (SlotActions[i])
            EnhancedInput->BindAction(SlotActions[i], ETriggerEvent::Triggered, this, &AHktPlayerController::OnSlotAction, i);
    }
}

//-----------------------------------------------------------------------------
// Input Handlers
//-----------------------------------------------------------------------------

void AHktPlayerController::OnSubjectAction(const FInputActionValue& Value)
{
    if (IntentBuilderComponent)
    {
        IntentBuilderComponent->CreateSubjectAction();
    }
}

void AHktPlayerController::OnSlotAction(const FInputActionValue& Value, int32 SlotIndex)
{
    if (!IntentBuilderComponent)
        return;

    IntentBuilderComponent->CreateCommandAction(SlotIndex);
    
    // Target이 필요 없으면 바로 제출
    if (!IntentBuilderComponent->IsTargetRequired() && IntentBuilderComponent->IsReadyToSubmit())
    {
        SendIntent();
    }
}

void AHktPlayerController::OnTargetAction(const FInputActionValue& Value)
{
    if (!IntentBuilderComponent)
        return;

    IntentBuilderComponent->CreateTargetAction();
    
    // Target 설정 완료 시 바로 제출
    if (IntentBuilderComponent->IsReadyToSubmit())
    {
        SendIntent();
    }
}

void AHktPlayerController::OnZoom(const FInputActionValue& Value)
{
    // Zoom logic...
}

bool AHktPlayerController::SendIntent()
{
    if (HasAuthority())
    {
        return false;
    }

    if (!IntentBuilderComponent)
    {
        return false;
    }

    FHktIntentEvent Event;
    if (!IntentBuilderComponent->SubmitIntent(Event))
    {
        return false;
    }

    Server_ReceiveIntent(Event);

    return true;
}

// === C2S RPC ===

bool AHktPlayerController::Server_ReceiveIntent_Validate(const FHktIntentEvent& Event)
{
    return Event.IsValid();
}

void AHktPlayerController::Server_ReceiveIntent_Implementation(const FHktIntentEvent& Event)
{
    if (AHktGameMode* GM = GetWorld()->GetAuthGameMode<AHktGameMode>())
    {
        GM->PushIntent(Event);
    }
}

// === S2C RPC ===

void AHktPlayerController::SendBatchToOwningClient(const FHktFrameBatch& Batch)
{
    if (HasAuthority())
    {
        Client_ReceiveBatch(Batch);
    }
}

void AHktPlayerController::Client_ReceiveBatch_Implementation(const FHktFrameBatch& Batch)
{
    if (!VisibleStashComponent)
    {
        return;
    }

    // 1. 제거된 엔티티
    for (FHktEntityId EntityId : Batch.RemovedEntities)
    {
        VisibleStashComponent->FreeEntity(EntityId);
    }

    // 2. 새 스냅샷 적용
    for (const FHktEntitySnapshot& Snapshot : Batch.Snapshots)
    {
        VisibleStashComponent->ApplyEntitySnapshot(Snapshot);
    }

    // 3. 이벤트 실행 (VMProcessor)
    if (VMProcessorComponent && VMProcessorComponent->IsInitialized())
    {
        // 모든 이벤트를 VMProcessor에 알림
        VMProcessorComponent->NotifyIntentEvents(Batch.FrameNumber, Batch.Events);
    }
}