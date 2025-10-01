#include "HktClientCoreSubsystem.h"
#include "HktRpcProxy.h"
#include "HktRpcTraits.h"
#include "HktGraph.h"
#include "HktDef.h"
#include "HktPacketTypes.h"

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
    RpcProxy = MakeShared<FHktRpcProxy>(TEXT("127.0.0.1:50051"));
    Graph = MakeShared<FHktGraph>();
}

void UHktClientCoreSubsystem::Deinitialize()
{
    RpcProxy.Reset();
    Graph.Reset();
    Super::Deinitialize();
}

void UHktClientCoreSubsystem::SyncGroup(int64 PlayerId, int64 GroupId)
{
    if (RpcProxy)
    {
        RpcProxy->SyncGroup(PlayerId, GroupId,
            [this](TUniquePtr<IHktBehavior> Response)
            {
                if (Response->GetTypeId() == GetBehaviorTypeId<FDestroyPacket>())
                {
                    OnBehaviorDestroyed.Broadcast(*Response);
                    Graph->RemoveBehavior(Response->GetBehaviorId());
                }
                else
                {
                    IHktBehavior& Behavior = Graph->AddBehavior(MoveTemp(Response));
                    OnBehaviorCreated.Broadcast(Behavior);
                }
            });
    }
}

void UHktClientCoreSubsystem::ExecuteBehavior(int64 SubjectId, int32 BehaviorTypeId, const TArray<uint8>& Bytes)
{
    if (RpcProxy)
    {
		constexpr int64 GroupId = 0; // Ŭ���̾�Ʈ���� �׷� ID�� �����ϴ� ������ �ʿ��� �� �ֽ��ϴ�.
        RpcProxy->ExecuteBehavior(GroupId, SubjectId, BehaviorTypeId, Bytes);
    }
}
