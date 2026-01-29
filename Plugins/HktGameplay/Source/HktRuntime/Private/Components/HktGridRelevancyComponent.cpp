#include "HktGridRelevancyComponent.h"
#include "HktPlayerController.h"
#include "GameFramework/Pawn.h"

UHktGridRelevancyComponent::UHktGridRelevancyComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

// === 클라이언트 관리 ===

void UHktGridRelevancyComponent::RegisterClient(AHktPlayerController* Client)
{
    if (!Client)
    {
        return;
    }

    for (const auto& WeakPC : RegisteredClients)
    {
        if (WeakPC.Get() == Client)
        {
            return;
        }
    }

    RegisteredClients.Add(Client);
    
    FHktPlayerGridCache& Cache = PlayerCaches.Add(Client);
    Cache.bIsDirty = true;

    UE_LOG(LogTemp, Log, TEXT("GridRelevancy: Registered client %s"), *Client->GetName());
}

void UHktGridRelevancyComponent::UnregisterClient(AHktPlayerController* Client)
{
    if (!Client)
    {
        return;
    }

    PlayerCaches.Remove(Client);
    RegisteredClients.RemoveAll([Client](const TWeakObjectPtr<AHktPlayerController>& WeakPC)
    {
        return !WeakPC.IsValid() || WeakPC.Get() == Client;
    });

    UE_LOG(LogTemp, Log, TEXT("GridRelevancy: Unregistered client %s"), *Client->GetName());
}

// === 업데이트 ===

void UHktGridRelevancyComponent::UpdateRelevancy(float DeltaTime)
{
    // 유효한 클라이언트 갱신
    ValidClients.Reset();
    
    for (int32 i = RegisteredClients.Num() - 1; i >= 0; --i)
    {
        AHktPlayerController* PC = RegisteredClients[i].Get();
        if (!PC)
        {
            RegisteredClients.RemoveAt(i);
            continue;
        }
        ValidClients.Add(PC);
    }

    // 각 플레이어 그리드 업데이트
    for (AHktPlayerController* PC : ValidClients)
    {
        FHktPlayerGridCache* Cache = PlayerCaches.Find(PC);
        if (!Cache)
        {
            continue;
        }

        FVector CurrentLocation = GetPlayerLocation(PC);

        float DistSq = FVector::DistSquared(CurrentLocation, Cache->LastLocation);
        if (DistSq > FMath::Square(MovementThreshold))
        {
            FIntPoint NewCell = LocationToCell(CurrentLocation);
            if (NewCell != Cache->CurrentCell)
            {
                Cache->bIsDirty = true;
            }
            Cache->LastLocation = CurrentLocation;
        }

        if (Cache->bIsDirty)
        {
            UpdatePlayerSubscription(PC, *Cache);
            Cache->bIsDirty = false;
        }
    }

    // 무효 캐시 정리
    TArray<AHktPlayerController*> ToRemove;
    for (auto& Pair : PlayerCaches)
    {
        if (!ValidClients.Contains(Pair.Key))
        {
            ToRemove.Add(Pair.Key);
        }
    }
    for (AHktPlayerController* PC : ToRemove)
    {
        PlayerCaches.Remove(PC);
    }
}

void UHktGridRelevancyComponent::UpdatePlayerSubscription(AHktPlayerController* PC, FHktPlayerGridCache& Cache)
{
    FVector Location = GetPlayerLocation(PC);
    FIntPoint NewCenterCell = LocationToCell(Location);

    // 새 구독 셀 계산
    Cache.SubscribedCellSet.Empty();
    for (int32 X = -InterestRadius; X <= InterestRadius; ++X)
    {
        for (int32 Y = -InterestRadius; Y <= InterestRadius; ++Y)
        {
            Cache.SubscribedCellSet.Add(FIntPoint(NewCenterCell.X + X, NewCenterCell.Y + Y));
        }
    }

    Cache.CurrentCell = NewCenterCell;
}

// === Relevancy 조회 ===

FIntPoint UHktGridRelevancyComponent::LocationToCell(const FVector& Location) const
{
    return FIntPoint(
        FMath::FloorToInt(Location.X / CellSize),
        FMath::FloorToInt(Location.Y / CellSize)
    );
}

bool UHktGridRelevancyComponent::IsClientInterestedInCell(AHktPlayerController* Client, FIntPoint Cell) const
{
    if (const FHktPlayerGridCache* Cache = PlayerCaches.Find(Client))
    {
        return Cache->SubscribedCellSet.Contains(Cell);
    }
    return false;
}

void UHktGridRelevancyComponent::GetRelevantClientsAtLocation(
    const FVector& Location,
    TArray<AHktPlayerController*>& OutRelevantClients)
{
    OutRelevantClients.Reset();

    FIntPoint Cell = LocationToCell(Location);

    for (AHktPlayerController* PC : ValidClients)
    {
        if (IsClientInterestedInCell(PC, Cell))
        {
            OutRelevantClients.Add(PC);
        }
    }
}

// === 헬퍼 ===

FVector UHktGridRelevancyComponent::GetPlayerLocation(AHktPlayerController* PC) const
{
    if (!PC)
    {
        return FVector::ZeroVector;
    }

    if (AActor* ViewTarget = PC->GetViewTarget())
    {
        return ViewTarget->GetActorLocation();
    }

    if (APawn* Pawn = PC->GetPawn())
    {
        return Pawn->GetActorLocation();
    }

    return FVector::ZeroVector;
}