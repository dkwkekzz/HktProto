// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "HktIntentPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class AHktIntentPlayerState;
class UHktIntentBuilderComponent;

/**
 * 입력을 받아 Intent를 조립하고 PlayerState로 제출하는 컨트롤러.
 */
UCLASS()
class HKTGAME_API AHktIntentPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AHktIntentPlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    //-------------------------------------------------------------------------
    // Input Handlers
    //-------------------------------------------------------------------------
    
    /** 좌클릭: 주체(Subject) 선택 */
    void OnSubjectAction(const FInputActionValue& Value);

    /** 우클릭: 대상(Target) 선택 및 제출 */
    void OnTargetAction(const FInputActionValue& Value);

    /** 숫자키: 명령(Command) 선택 */
    void OnSlotAction(const FInputActionValue& Value, int32 SlotIndex);

    void OnZoom(const FInputActionValue& Value);

protected:
    /** Intent 빌더 컴포넌트 (클라이언트 로컬 입력 조립용) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hkt|Components")
    TObjectPtr<UHktIntentBuilderComponent> IntentBuilder;

    //-------------------------------------------------------------------------
    // Config
    //-------------------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "Hkt|Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, Category = "Hkt|Input")
    TObjectPtr<UInputAction> SubjectAction;

    UPROPERTY(EditDefaultsOnly, Category = "Hkt|Input")
    TObjectPtr<UInputAction> TargetAction;

    UPROPERTY(EditDefaultsOnly, Category = "Hkt|Input")
    TObjectPtr<UInputAction> ZoomAction;

    UPROPERTY(EditDefaultsOnly, Category = "Hkt|Input")
    TArray<TObjectPtr<UInputAction>> SlotActions;
};
