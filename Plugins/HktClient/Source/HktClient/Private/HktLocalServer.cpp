#include "HktLocalServer.h"
#include "HktReliableUdpServer.h"

FHktLocalServer::FHktLocalServer()
{
}

FHktLocalServer::~FHktLocalServer()
{
    Stop();
}

void FHktLocalServer::Start()
{
    if (Server)
    {
        UE_LOG(LogTemp, Warning, TEXT("Server is already running."));
        return;
    }

    const uint16 Port = HktReliableUdp::ServerPort;
    Server = MakeUnique<FHktReliableUdpServer>(Port);
    Server->Start();

    UE_LOG(LogTemp, Log, TEXT("Local server started on port %d"), Port);
}

void FHktLocalServer::Stop()
{
    if (!Server)
    {
        return;
    }

    Server->Stop();
    Server.Reset();
    UE_LOG(LogTemp, Log, TEXT("Local server stopped."));
}

bool FHktLocalServer::IsRunning() const
{
    return Server.IsValid();
}

void FHktLocalServer::Tick(float DeltaTime)
{
    if (Server)
    {
        Server->Tick();
    }
}

bool FHktLocalServer::IsTickable() const
{
    return IsRunning();
}

TStatId FHktLocalServer::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(FHktLocalServer, STATGROUP_Tickables);
}

