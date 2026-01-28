// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktPlayerController.h"
#include "HktIntentBuilderComponent.h"
#include "HktIntentEventComponent.h"
#include "HktVisibleStashComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

AHktPlayerController::AHktPlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    IntentEventComponent = CreateDefaultSubobject<UHktIntentEventComponent>(TEXT("IntentEventComponent"));
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
    
    // 클라이언트에서만 VisibleStashComponent 생성
    if (!HasAuthority())
    {
        IntentBuilderComponent = NewObject<UHktIntentBuilderComponent>(this, TEXT("IntentBuilderComponent"));
        IntentBuilderComponent->RegisterComponent();
        
        VisibleStashComponent = NewObject<UHktVisibleStashComponent>(this, TEXT("VisibleStashComponent"));
        VisibleStashComponent->RegisterComponent();

        // IntentEventComponent → VisibleStashComponent 연결
        if (IntentEventComponent)
        {
            IntentEventComponent->OnClientReceivedBatch.AddDynamic(
                this, &AHktPlayerController::OnFrameBatchReceived);
        }

        UE_LOG(LogTemp, Log, TEXT("HktPlayerController: Client initialized with VisibleStashComponent"));
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
        FHktIntentEvent Event;
        if (IntentBuilderComponent->SubmitIntent(Event))
        {
            IntentEventComponent->CommitIntent(Event);
        }
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
        FHktIntentEvent Event;
        if (IntentBuilderComponent->SubmitIntent(Event))
        {
            IntentEventComponent->SendIntentToServer(Event);
        }
    }
}

void AHktPlayerController::OnZoom(const FInputActionValue& Value)
{
    // Zoom logic...
}

void AHktPlayerController::OnFrameBatchReceived(const FHktIntentEventBatch& Batch)
{
    if (!VisibleStashComponent)
    {
        return;
    }

    // 1. 각 이벤트의 스냅샷 적용
    for (const FHktIntentEvent& Event : Batch.Events)
    {
        VisibleStashComponent->ApplySnapshots(Event.AttachedSnapshots);
    }

    // 2. VMProcessor에서 이벤트 실행 (TODO)
    // for (const FHktFrameEvent& FrameEvent : Batch.Events)
    // {
    //     VMProcessor->QueueIntent(FrameEvent.Intent);
    // }

    UE_LOG(LogTemp, Verbose, TEXT("HktPlayerController: Applied Frame %d with %d events"),
        Batch.FrameNumber, Batch.Num());
}