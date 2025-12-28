// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "HktInputContexts.h" // Context Interfaces
#include "HktIntentPlayerController.generated.h"

class UHktIntentComponent;
class UHktActionDataAsset;
class UInputMappingContext;
class UInputAction;
class UNiagaraSystem;

/**
 * 입력을 받아 Context를 생성(Factory)하고 조립(Staging)하는 컨트롤러.
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
    
    /** 좌클릭: 주체(Subject) 컨텍스트 생성 및 저장 */
    void HandleSubjectAction(const FInputActionValue& Value);

    /** 우클릭: 대상(Target) 컨텍스트 생성 및 최종 제출(Submit) */
    void HandleTargetAction(const FInputActionValue& Value);

    /** 숫자키: 명령(Command) 컨텍스트 생성 및 저장 */
    void HandleSlotAction(const FInputActionValue& Value, int32 SlotIndex);

    void HandleZoom(const FInputActionValue& Value);

    //-------------------------------------------------------------------------
    // Helper
    //-------------------------------------------------------------------------
    bool GetHitUnderCursor(FHitResult& OutHitResult) const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hkt|Components")
    TObjectPtr<UHktIntentComponent> IntentComponent;

    //-------------------------------------------------------------------------
    // Staged Contexts (현재 조립 중인 의도 재료들)
    //-------------------------------------------------------------------------

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "Hkt|Context")
    TScriptInterface<IHktSubjectContext> PendingSubjectContext;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "Hkt|Context")
    TScriptInterface<IHktCommandContext> PendingCommandContext;

    // TargetContext는 보통 즉발성(우클릭 순간)이므로 멤버로 저장할 수도, 
    // 로컬변수로 처리할 수도 있습니다. 여기서는 제출 시점에 생성합니다.

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