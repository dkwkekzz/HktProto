// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "HktPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class AHktPlayerState;
class UHktIntentBuilderComponent;
class UHktVisibleStashComponent;
struct FHktIntentEventBatch;

/**
 * 입력을 받아 Intent를 조립하고 PlayerState로 제출하는 컨트롤러.
 */
UCLASS()
class HKTRUNTIME_API AHktPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AHktPlayerController();

    // === 컴포넌트 접근 ===
    
    UFUNCTION(BlueprintPure, Category = "Hkt")
    UHktIntentBuilderComponent* GetIntentBuilderComponent() const { return IntentBuilderComponent; }

    UFUNCTION(BlueprintPure, Category = "Hkt")
    UHktIntentEventComponent* GetIntentEventComponent() const { return IntentEventComponent; }

    UFUNCTION(BlueprintPure, Category = "Hkt")
    UHktVisibleStashComponent* GetVisibleStashComponent() const { return VisibleStashComponent; }

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

    void OnFrameBatchReceived(const FHktIntentEventBatch& Batch);

protected:
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
    
    /** Intent 빌더 컴포넌트 (클라이언트 로컬 입력 조립용) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hkt|Components")
    TObjectPtr<UHktIntentBuilderComponent> IntentBuilderComponent;

	/** Intent 전송 컴포넌트 (네트워크 복제 담당) */
	UPROPERTY(VisibleAnywhere, Category = "Hkt|Components")
	TObjectPtr<UHktIntentEventComponent> IntentEventComponent;

    // VisibleStashComponent (클라이언트 전용)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hkt")
    TObjectPtr<UHktVisibleStashComponent> VisibleStashComponent;
};
