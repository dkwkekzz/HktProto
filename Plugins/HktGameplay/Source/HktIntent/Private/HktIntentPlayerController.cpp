// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktIntentPlayerController.h"
#include "HktIntentComponent.h"
#include "HktInputContexts.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

AHktIntentPlayerController::AHktIntentPlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    IntentComponent = CreateDefaultSubobject<UHktIntentComponent>(TEXT("IntentComponent"));
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
        EnhancedInput->BindAction(SubjectAction, ETriggerEvent::Triggered, this, &AHktIntentPlayerController::HandleSubjectAction);

    if (TargetAction)
        EnhancedInput->BindAction(TargetAction, ETriggerEvent::Triggered, this, &AHktIntentPlayerController::HandleTargetAction);

    if (ZoomAction)
        EnhancedInput->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &AHktIntentPlayerController::HandleZoom);

    for (int32 i = 0; i < SlotActions.Num(); ++i)
    {
        if (SlotActions[i])
            EnhancedInput->BindAction(SlotActions[i], ETriggerEvent::Started, this, &AHktIntentPlayerController::HandleSlotAction, i);
    }
}

//-----------------------------------------------------------------------------
// Input Handlers - Build Contexts
//-----------------------------------------------------------------------------

void AHktIntentPlayerController::HandleSubjectAction(const FInputActionValue& Value)
{
    FHitResult Hit;
    if (GetHitUnderCursor(Hit))
    {
        // 1. Subject Context 생성: "클릭으로 주체를 정하겠다"는 의도를 객체화
        UHktSubjectContext_ByClick* NewContext = NewObject<UHktSubjectContext_ByClick>(this);
        NewContext->Initialize(Hit);
        
        // 2. 저장 (Staging)
        PendingSubjectContext = NewContext;
        
        // 새로운 주체가 선택되었으므로 이전 명령은 초기화하는 것이 자연스러움
        PendingCommandContext = nullptr;

        // 로그: 디버깅용
        // TArray<FHktUnitHandle> Subjects = NewContext->ResolveSubjects(GetWorld());
        // UE_LOG(LogTemp, Log, TEXT("Subject Context Created. Resolved Units: %d"), Subjects.Num());
    }
    else
    {
        PendingSubjectContext = nullptr;
        PendingCommandContext = nullptr;
    }
}

void AHktIntentPlayerController::HandleSlotAction(const FInputActionValue& Value, int32 SlotIndex)
{
    // 주체가 없으면 명령을 내릴 수 없음 (또는 전역 스킬일 경우 예외 처리 가능)
    if (!PendingSubjectContext) 
    {
        return;
    }

    // 1. Command Context 생성: "이 에셋을 사용하겠다"는 의도를 객체화
    UHktCommandContext_BySlot* NewContext = NewObject<UHktCommandContext_BySlot>(this);
    NewContext->Initialize(PendingSubjectContext, SlotIndex);
    // 2. 저장 (Staging)
    PendingCommandContext = NewContext;
    
	if (PendingCommandContext->IsRequiredTarget() == false)
	{
		if (IntentComponent)
		{
			IntentComponent->SubmitIntent(PendingSubjectContext, PendingCommandContext, nullptr);
		}
	}
	
    UE_LOG(LogTemp, Log, TEXT("Command Context Staged: %s"), *GetNameSafe(SelectedAction));
}

void AHktIntentPlayerController::HandleTargetAction(const FInputActionValue& Value)
{
    // Submit 조건 검사: 주체와 명령이 모두 준비되었는가?
    if (!PendingSubjectContext || !PendingCommandContext)
    {
        return;
    }

    FHitResult Hit;
    if (GetHitUnderCursor(Hit))
    {
        // 1. Target Context 생성: "여기를 때리겠다"는 의도를 객체화
        UHktTargetContext_ByClick* TargetContext = NewObject<UHktTargetContext_ByClick>(this);
        TargetContext->Initialize(Hit);

        // 2. 최종 제출 (Submit)
        // 컴포넌트에게 "이 주체(S)가, 이 명령(C)을, 여기(T)에 수행한다"고 전달
        if (IntentComponent)
        {
            IntentComponent->SubmitIntent(PendingSubjectContext, PendingCommandContext, TargetContext);
            
            // 제출 후 명령만 초기화할지, 주체도 초기화할지는 기획에 따라 결정 (여기서는 명령만 리셋)
            // PendingCommandContext = nullptr; 
        }
    }
}

void AHktIntentPlayerController::HandleZoom(const FInputActionValue& Value)
{
    // Zoom logic...
}

bool AHktIntentPlayerController::GetHitUnderCursor(FHitResult& OutHitResult) const
{
    return GetHitResultUnderCursor(ECC_Visibility, false, OutHitResult);
}