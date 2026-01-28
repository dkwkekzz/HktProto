#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HktCoreTypes.h"
#include "HktGameMode.generated.h"

class UHktIntentEventComponent;
class UHktMasterStashComponent;
class AHktPlayerController;

/**
 * AHktGameMode
 * 
 * 서버에서 IntentEvent를 배치 처리하고 각 클라이언트에 분배
 * - 틱마다 수신된 Intent들을 모아서 처리
 * - 각 클라이언트의 Relevancy를 체크하여 필요한 스냅샷 첨부
 * - 배치로 묶어서 전송
 */
UCLASS()
class HKTRUNTIME_API AHktGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
	static AHktGameMode* Get(const UObject* WorldContextObject);

    AHktGameMode();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    
    // 플레이어 로그인/로그아웃
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

    // Intent 수신 (델리게이트 핸들러)
    UFUNCTION()
    void OnServerReceivedIntent(const FHktIntentEvent& Event);

    // MasterStash 접근
    UMasterStashComponent* GetMasterStash() const { return MasterStash; }

protected:
    // 틱마다 배치 처리
    virtual void ProcessIntentBatch();

    // 특정 클라이언트에게 이벤트 전송 준비
    FHktIntentEvent PrepareEventForClient(
        AHktPlayerController* ClientPC, 
        const FHktIntentEvent& OriginalEvent
    );

    // 엔티티가 클라이언트에게 관련있는지 체크
    bool IsEntityRelevantToClient(AHktPlayerController* ClientPC, FHktEntityId EntityId) const;

    // 이벤트가 클라이언트에게 관련있는지 체크
    bool IsEventRelevantToClient(AHktPlayerController* ClientPC, const FHktIntentEvent& Event) const;

private:
    // 등록된 PlayerController 목록
    UPROPERTY()
    TArray<TWeakObjectPtr<AHktPlayerController>> RegisteredControllers;

    // 이번 틱에 수신된 Intent 목록
    TArray<FHktIntentEvent> PendingIntents;

    // 현재 서버 틱
    int64 CurrentServerTick = 0;

    // MasterStash 컴포넌트 (모든 엔티티 데이터)
    UPROPERTY()
    UHktMasterStashComponent* MasterStash;

    // Intent ID 카운터
    uint32 NextIntentId = 1;
};