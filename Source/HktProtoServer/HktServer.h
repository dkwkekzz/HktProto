#pragma once

#include "CoreMinimal.h"
#include <string>

class FHktRpcService;

class FHktServer
{
public:
    FHktServer();
    ~FHktServer();

    void Run(const std::string& server_address, size_t thread_count);
    void Shutdown();

private:
    TUniquePtr<FHktRpcService> RpcService;
};
