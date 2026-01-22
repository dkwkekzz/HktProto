// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktIntentPlayerController.h"
#include "HktIntentBuilderComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

AHktIntentPlayerController::AHktIntentPlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    IntentBuilder = CreateDefaultSubobject<UHktIntentBuilderComponent>(TEXT("IntentBuilder"));
}

void AHktIntentPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (DefaultMappingContext)
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void AHktIntentPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
    if (!EnhancedInput) 
        return;

    if (SubjectAction)
        EnhancedInput->BindAction(SubjectAction, ETriggerEvent::Triggered, this, &AHktIntentPlayerController::OnSubjectAction);

    if (TargetAction)
        EnhancedInput->BindAction(TargetAction, ETriggerEvent::Triggered, this, &AHktIntentPlayerController::OnTargetAction);

    if (ZoomAction)
        EnhancedInput->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &AHktIntentPlayerController::OnZoom);

    for (int32 i = 0; i < SlotActions.Num(); ++i)
    {
        if (SlotActions[i])
            EnhancedInput->BindAction(SlotActions[i], ETriggerEvent::Triggered, this, &AHktIntentPlayerController::OnSlotAction, i);
    }
}

//-----------------------------------------------------------------------------
// Input Handlers
//-----------------------------------------------------------------------------

void AHktIntentPlayerController::OnSubjectAction(const FInputActionValue& Value)
{
    if (IntentBuilder)
    {
        IntentBuilder->CreateSubjectAction();
    }
}

void AHktIntentPlayerController::OnSlotAction(const FInputActionValue& Value, int32 SlotIndex)
{
    if (!IntentBuilder)
        return;

    IntentBuilder->CreateCommandAction(SlotIndex);
    
    // Target이 필요 없으면 바로 제출
    if (!IntentBuilder->IsTargetRequired() && IntentBuilder->IsReadyToSubmit())
    {
        IntentBuilder->SubmitIntent();
    }
}

void AHktIntentPlayerController::OnTargetAction(const FInputActionValue& Value)
{
    if (!IntentBuilder)
        return;

    IntentBuilder->CreateTargetAction();
    
    // Target 설정 완료 시 바로 제출
    if (IntentBuilder->IsReadyToSubmit())
    {
        IntentBuilder->SubmitIntent();
    }
}

void AHktIntentPlayerController::OnZoom(const FInputActionValue& Value)
{
    // Zoom logic...
}
