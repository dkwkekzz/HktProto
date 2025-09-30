#include "HktClientCoreSubsystem.h"
#include "HktRpcProxy.h"
#include "HktRpcTraits.h"
#include "HktGraph.h"
#include "HktDef.h"
#include "HktPacketTypes.h"

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

void UHktClientCoreSubsystem::SyncGroup(int64 SubjectId, int64 GroupId)
{
    if (RpcProxy)
    {
        RpcProxy->SyncGroup(SubjectId, GroupId,
            [this](TUniquePtr<IHktBehavior> Response)
            {
                if (Response->GetTypeId() == GetBehaviorTypeId<FDestroyPacket>())
                {
                    OnDestroyedBehavior.Broadcast(*Response);
                    Graph->RemoveBehavior(Response->GetBehaviorId());
                }
                else
                {
                    IHktBehavior& Behavior = Graph->AddBehavior(MoveTemp(Response));
                    OnCreatedBehavior.Broadcast(Behavior);
                }
            });
    }
}

void UHktClientCoreSubsystem::ExecuteBehavior(int64 SubjectId, int32 BehaviorTypeId, const TArray<uint8>& Bytes)
{
    if (RpcProxy)
    {
		constexpr int64 GroupId = 0; // 클라이언트에서 그룹 ID를 관리하는 로직이 필요할 수 있습니다.
        RpcProxy->ExecuteBehavior(GroupId, SubjectId, BehaviorTypeId, Bytes);
    }
}
