#include "HktClientCoreSubsystem.h"
#include "HktGraph.h"
#include "HktReliableUdpClient.h"

UHktClientCoreSubsystem* UHktClientCoreSubsystem::Get(UWorld* InWorld)
{
    if (InWorld)
    {
        if (UGameInstance* GameInstance = InWorld->GetGameInstance())
        {
            return GameInstance->GetSubsystem<UHktClientCoreSubsystem>();
        }
    }

    return nullptr;
}

void UHktClientCoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    NetClient = MakeShared<FHktReliableUdpClient>();
    Graph = MakeShared<FHktGraph>();
}

void UHktClientCoreSubsystem::Deinitialize()
{
    NetClient.Reset();
    Graph.Reset();
    Super::Deinitialize();
}

void UHktClientCoreSubsystem::Tick(float DeltaTime)
{
    if (NetClient)
    {
        NetClient->Tick();

		TArray<uint8> Bytes;
        while (NetClient->Poll(Bytes))
        {
            FHktBehaviorResponseHeader ResponseHeader;
            if (auto NewBehavior = FHktBehaviorFactory::CreateBehavior(ResponseHeader))
            {
                const auto& BehaviorRef = Graph->AddBehavior(MoveTemp(NewBehavior));
                OnBehaviorCreated.Broadcast(BehaviorRef);
            }
            else
            {
                if (const auto* BehaviorPtr = Graph->FindBehavior(ResponseHeader.BehaviorInstanceId))
                {
					OnBehaviorDestroyed.Broadcast(*BehaviorPtr);
                    Graph->RemoveBehavior(*BehaviorPtr);
                }
            }
        }
	}
}

bool UHktClientCoreSubsystem::IsTickable() const
{
    if (NetClient)
    {
        return NetClient->IsConnected();
    }

    return false;
}

void UHktClientCoreSubsystem::Connect()
{
    if (NetClient)
    {
        NetClient->Connect(TEXT("127.0.0.1"), HktReliableUdp::ServerPort);
    }
}

void UHktClientCoreSubsystem::Disconnect()
{
    if (NetClient)
    {
        NetClient->Disconnect();
    }
}

void UHktClientCoreSubsystem::SendBytes(const TArray<uint8>& Bytes)
{
    if (NetClient)
    {
        NetClient->Send(Bytes);
    }
}
