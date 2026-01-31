// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "HktCoreTypes.h"
#include "HktRuntimeInterfaces.h"
#include "HktPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UHktInputAction;
class UHktIntentBuilderComponent;
class UHktVisibleStashComponent;
class UHktVMProcessorComponent;
struct FHktFrameBatch;

/**
 * FHktClientRelevancy - 클라이언트의 Relevancy 상태
 * 
 * "현재 Relevancy 영역 안에 있는 엔티티"를 추적
 * Relevancy를 벗어나면 제거, 다시 들어오면 스냅샷 전송
 */
USTRUCT()
struct HKTRUNTIME_API FHktClientRelevancy
{
    GENERATED_BODY()

    // 현재 Relevancy 안에 있는 엔티티
    TSet<FHktEntityId> RelevantEntities;

    // 이번 프레임에 새로 진입한 엔티티 (스냅샷 필요)
    TArray<FHktEntityId> EnteredEntities;

    // 이번 프레임에 벗어난 엔티티 (클라에게 제거 알림)
    TArray<FHktEntityId> ExitedEntities;

    bool IsRelevant(FHktEntityId EntityId) const
    {
        return RelevantEntities.Contains(EntityId);
    }

    // Relevancy 진입
    void EnterRelevancy(FHktEntityId EntityId)
    {
        if (!RelevantEntities.Contains(EntityId))
        {
            RelevantEntities.Add(EntityId);
            EnteredEntities.Add(EntityId);
        }
    }

    // Relevancy 이탈
    void ExitRelevancy(FHktEntityId EntityId)
    {
        if (RelevantEntities.Remove(EntityId) > 0)
        {
            ExitedEntities.Add(EntityId);
        }
    }

    // 프레임 시작 시 버퍼 초기화
    void BeginFrame()
    {
        EnteredEntities.Reset();
        ExitedEntities.Reset();
    }

    void Reset()
    {
        RelevantEntities.Empty();
        EnteredEntities.Empty();
        ExitedEntities.Empty();
    }
};

/**
 * 입력을 받아 Intent를 조립하고 PlayerState로 제출하는 컨트롤러.
 */
UCLASS()
class HKTRUNTIME_API AHktPlayerController : public APlayerController, public IHktModelProvider
{
    GENERATED_BODY()

public:
    AHktPlayerController();

    // === Intent 전송 (클라이언트) ===
    
    UFUNCTION(BlueprintCallable, Category = "Hkt")
    bool SendIntent();

    // === C2S RPC ===
    
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_ReceiveIntent(const FHktIntentEvent& Event);

    // === S2C RPC ===
    
    void SendBatchToOwningClient(const FHktFrameBatch& Batch);

    UFUNCTION(Client, Reliable)
    void Client_ReceiveBatch(const FHktFrameBatch& Batch);

    // === Relevancy (서버 전용) ===
    
    FHktClientRelevancy& GetRelevancy() { return Relevancy; }
    const FHktClientRelevancy& GetRelevancy() const { return Relevancy; }

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    //-------------------------------------------------------------------------
    // Input Handlers
    //-------------------------------------------------------------------------

    void OnSubjectAction(const FInputActionValue& Value);
    void OnTargetAction(const FInputActionValue& Value);
    void OnSlotAction(const FInputActionValue& Value, int32 SlotIndex);
    void OnZoom(const FInputActionValue& Value);

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Hkt|Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, Category = "Hkt|Input")
    TObjectPtr<UInputAction> SubjectAction;

    UPROPERTY(EditDefaultsOnly, Category = "Hkt|Input")
    TObjectPtr<UInputAction> TargetAction;

    UPROPERTY(EditDefaultsOnly, Category = "Hkt|Input")
    TObjectPtr<UInputAction> ZoomAction;

    UPROPERTY(EditDefaultsOnly, Category = "Hkt|Input")
    TArray<TObjectPtr<UHktInputAction>> SlotActions;
    
    /** Intent 빌더 컴포넌트 (클라이언트 로컬 입력 조립용) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hkt|Components")
    TObjectPtr<UHktIntentBuilderComponent> IntentBuilderComponent;

    // VisibleStashComponent (클라이언트 전용)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hkt")
    TObjectPtr<UHktVisibleStashComponent> VisibleStashComponent;
    
    /** VM 프로세서 컴포넌트 (클라이언트 로컬 시뮬레이션 실행) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hkt")
    TObjectPtr<UHktVMProcessorComponent> VMProcessorComponent;
    
    FHktClientRelevancy Relevancy;

    //-------------------------------------------------------------------------
    // IHktControlProvider 델리게이트
    //-------------------------------------------------------------------------

    FOnHktSubjectChanged SubjectChangedDelegate;
    FOnHktTargetChanged TargetChangedDelegate;
    FOnHktCommandChanged CommandChangedDelegate;
    FOnHktIntentSubmitted IntentSubmittedDelegate;
    FOnHktWheelInput WheelInputDelegate;
    FOnHktEntityCreated EntityCreatedDelegate;
    FOnHktEntityDestroyed EntityDestroyedDelegate;

public:
    //-------------------------------------------------------------------------
    // IHktControlProvider 구현
    //-------------------------------------------------------------------------

    virtual IHktStashInterface* GetStashInterface() const override;
    virtual FHktEntityId GetSelectedSubject() const override;
    virtual FHktEntityId GetSelectedTarget() const override;
    virtual FVector GetTargetLocation() const override;
    virtual FGameplayTag GetSelectedCommand() const override;
    virtual bool IsIntentValid() const override;
    virtual FOnHktSubjectChanged& OnSubjectChanged() override { return SubjectChangedDelegate; }
    virtual FOnHktTargetChanged& OnTargetChanged() override { return TargetChangedDelegate; }
    virtual FOnHktCommandChanged& OnCommandChanged() override { return CommandChangedDelegate; }
    virtual FOnHktIntentSubmitted& OnIntentSubmitted() override { return IntentSubmittedDelegate; }
    virtual FOnHktWheelInput& OnWheelInput() override { return WheelInputDelegate; }
    virtual FOnHktEntityCreated& OnEntityCreated() override { return EntityCreatedDelegate; }
    virtual FOnHktEntityDestroyed& OnEntityDestroyed() override { return EntityDestroyedDelegate; }
};
