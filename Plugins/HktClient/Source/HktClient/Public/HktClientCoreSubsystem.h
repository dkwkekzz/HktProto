#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HktRpcTraits.h"
#include "HktClientCoreSubsystem.generated.h"

class FHktRpcProxy;

UCLASS()
class HKTCLIENT_API UHktClientCoreSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
    
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void Login(const hkt::AccountRequest& Request);
    void CreateBehavior(const hkt::CreateBehaviorRequest& Request);
    void DestroyBehavior(const hkt::DestroyBehaviorRequest& Request);

    DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLoginResponse, const hkt::AccountResponse& /* Response */, const grpc::Status& /* Status */);
    FOnLoginResponse OnLoginResponse;

    DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCreateBehaviorResponse, const hkt::CreateBehaviorResponse& /* Response */, const grpc::Status& /* Status */);
    FOnCreateBehaviorResponse OnCreateBehaviorResponse;

    DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDestroyBehaviorResponse, const hkt::DestroyBehaviorResponse& /* Response */, const grpc::Status& /* Status */);
    FOnDestroyBehaviorResponse OnDestroyBehaviorResponse;

    DECLARE_MULTICAST_DELEGATE_OneParam(FOnSyncPacket, const hkt::BehaviorPacket& /* Packet */);
    FOnSyncPacket OnSyncPacket;

private:
    void HandleSyncResponse(const hkt::SyncResponse& Response);

    TSharedPtr<FHktRpcProxy> RpcProxy;
};
