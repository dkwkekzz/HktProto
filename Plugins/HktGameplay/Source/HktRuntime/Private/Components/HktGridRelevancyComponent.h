#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HktRelevancyInterface.h"
#include "HktGridRelevancyComponent.generated.h"

class AHktPlayerController;

/**
 * 플레이어별 그리드 캐시 정보
 */
USTRUCT()
struct FHktPlayerGridCache
{
    GENERATED_BODY()

    FIntPoint CurrentCell = FIntPoint(INT_MAX, INT_MAX);
    FVector LastLocation = FVector::ZeroVector;
    bool bIsDirty = true;

    // 구독 중인 셀 (O(1) 조회용 Set)
    TSet<FIntPoint> SubscribedCellSet;
};

/**
 * UHktGridRelevancyComponent
 * 
 * 그리드 기반 Relevancy 정책
 * - 클라이언트별 구독 셀 Set으로 O(1) 체크
 */
UCLASS(ClassGroup=(HktSimulation), meta=(BlueprintSpawnableComponent))
class HKTSIMULATION_API UHktGridRelevancyComponent : public UActorComponent, public IHktRelevancyProvider
{
    GENERATED_BODY()

public:
    UHktGridRelevancyComponent();

    // === IHktRelevancyProvider 구현 ===
    
    virtual void RegisterClient(AHktPlayerController* Client) override;
    virtual void UnregisterClient(AHktPlayerController* Client) override;
    virtual const TArray<AHktPlayerController*>& GetAllClients() const override { return ValidClients; }
    
    virtual void GetRelevantClientsAtLocation(
        const FVector& Location,
        TArray<AHktPlayerController*>& OutRelevantClients
    ) override;

    virtual void GetAllRelevantClients(TArray<AHktPlayerController*>& OutRelevantClients) override
    {
        OutRelevantClients = ValidClients;
    }

    virtual void UpdateRelevancy(float DeltaTime) override;

    // === 클라이언트 단위 조회 (병렬 처리용) ===
    
    // 위치 → 셀 변환 (public)
    FIntPoint LocationToCell(const FVector& Location) const;

    // 클라이언트가 해당 셀에 관심 있는지 O(1) 체크
    bool IsClientInterestedInCell(AHktPlayerController* Client, FIntPoint Cell) const;

    // 글로벌 이벤트용
    bool IsClientInterestedInGlobal(AHktPlayerController* Client) const { return true; }

    // === 설정 ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hkt|Grid")
    float CellSize = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hkt|Grid")
    int32 InterestRadius = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hkt|Grid")
    float MovementThreshold = 100.0f;

protected:
    FVector GetPlayerLocation(AHktPlayerController* PC) const;
    void UpdatePlayerSubscription(AHktPlayerController* PC, FHktPlayerGridCache& Cache);

private:
    TArray<TWeakObjectPtr<AHktPlayerController>> RegisteredClients;
    TArray<AHktPlayerController*> ValidClients;
    TMap<AHktPlayerController*, FHktPlayerGridCache> PlayerCaches;
};