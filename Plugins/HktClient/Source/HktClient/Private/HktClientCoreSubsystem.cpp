#include "HktClientCoreSubsystem.h"
#include "HktRpcProxy.h"

void UHktClientCoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    RpcProxy = MakeShared<FHktRpcProxy>(TEXT("127.0.0.1:50051"));
}

void UHktClientCoreSubsystem::Deinitialize()
{
    RpcProxy.Reset();
    Super::Deinitialize();
}
