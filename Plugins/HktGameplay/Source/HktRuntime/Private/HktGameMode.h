#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HktRuntimeTypes.h"
#include "HktGameMode.generated.h"

class UHktMasterStashComponent;
class UHktGridRelevancyComponent;
class UHktVMProcessorComponent;
class AHktPlayerController;

/**
 * AHktGameMode
 * 
 * 서버 GameMode
 * - 클라이언트 단위 병렬 처리
 * - 각 클라이언트가 자신에게 관련된 이벤트만 필터링
 */
UCLASS()
class HKTRUNTIME_API AHktGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AHktGameMode();

    UFUNCTION(BlueprintPure, Category = "Hkt")
    int32 GetFrameNumber() const { return FrameNumber; }

    void PushIntent(const FHktIntentEvent& Event);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

    void ProcessFrame();
    void ProcessFrameEventCell();
    void ProcessFrameClientBatch(AHktPlayerController*& PC, FHktFrameBatch& Batch);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hkt")
    UHktMasterStashComponent* MasterStash;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hkt")
    UHktGridRelevancyComponent* GridRelevancy;

    /** VM 프로세서 컴포넌트 (서버 시뮬레이션 실행) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hkt")
    UHktVMProcessorComponent* VMProcessor;

private:
    int32 FrameNumber = 0;

    // Intent 수집 (락 보호)
    TArray<FHktIntentEvent> CollectedIntents;
    FCriticalSection IntentLock;

    // 프레임 처리용 (매 프레임 재사용)
    TArray<FHktIntentEvent> FrameIntents;
    
    // 이벤트별 셀 인덱스 캐시 (병렬 접근용)
    struct FEventCellInfo
    {
        FIntPoint Cell;
        bool bIsGlobal;
        bool bHasValidLocation;
    };
    TArray<FEventCellInfo> EventCellCache;
};