#include "HktServer.h"
#include "HktGrpc.h"
#include "HktRpcService.h"

FHktServer::FHktServer()
    : RpcService(MakeUnique<FHktRpcService>())
{
}

FHktServer::~FHktServer()
{
    Shutdown();
}

void FHktServer::Run(const std::string& server_address, size_t thread_count)
{
    check(RpcService);
    RpcService->Run(server_address, thread_count);
}

void FHktServer::Shutdown()
{
    if (RpcService)
    {
        RpcService->Shutdown();
        RpcService.Reset();
    }
}
