#include "HktClientCoreSubsystem.h"
#include "HktRpcProxy.h"

void UHktClientCoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    RpcProxy = MakeShared<FHktRpcProxy>(TEXT("127.0.0.1:50051"));
    if (RpcProxy.IsValid())
    {
        RpcProxy->OnSyncResponse.AddUObject(this, &UHktClientCoreSubsystem::HandleSyncResponse);
    }
}

void UHktClientCoreSubsystem::Deinitialize()
{
    if (RpcProxy.IsValid())
    {
        RpcProxy->OnSyncResponse.RemoveAll(this);
    }
    RpcProxy.Reset();
    Super::Deinitialize();
}

void UHktClientCoreSubsystem::Login(const hkt::AccountRequest& Request)
{
    if (!RpcProxy.IsValid()) return;

    RpcProxy->Login(Request,
        [this](const hkt::AccountResponse& Response, const grpc::Status& Status)
        {
            RpcProxy->StartSync(Response.player_id(), 0);
            OnLoginResponse.Broadcast(Response, Status);
        }
    );
}

void UHktClientCoreSubsystem::CreateBehavior(const hkt::CreateBehaviorRequest& Request)
{
    if (!RpcProxy.IsValid()) return;

    RpcProxy->CreateBehavior(Request,
        [this](const hkt::CreateBehaviorResponse& Response, const grpc::Status& Status)
        {
            OnCreateBehaviorResponse.Broadcast(Response, Status);
        }
    );
}

void UHktClientCoreSubsystem::DestroyBehavior(const hkt::DestroyBehaviorRequest& Request)
{
    if (!RpcProxy.IsValid()) return;

    RpcProxy->DestroyBehavior(Request,
        [this](const hkt::DestroyBehaviorResponse& Response, const grpc::Status& Status)
        {
            OnDestroyBehaviorResponse.Broadcast(Response, Status);
        }
    );
}

void UHktClientCoreSubsystem::HandleSyncResponse(const hkt::SyncResponse& Response)
{
    if (Response.has_behavior_packet())
    {
        //const auto& Packet = Response.behavior_packet();
        //OnSyncPacket.Broadcast(Packet);
    }
}