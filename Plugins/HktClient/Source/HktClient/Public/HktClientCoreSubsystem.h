#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HktStructSerializer.h"
#include "HktClientCoreSubsystem.generated.h"

class IHktBehavior;
class FHktRpcProxy;
class FHktGraph;
struct FHktPacketBase;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSyncBehavior, IHktBehavior&/* Response */);

UCLASS()
class HKTCLIENT_API UHktClientCoreSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
    
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void SyncGroup(int64 SubjectId, int64 GroupId);

    template<typename TPacket>
    FORCEINLINE void ExecuteBehavior(int64 SubjectId, const TPacket& Packet) 
    { 
        ExecuteBehavior(SubjectId, GetBehaviorTypeId<TPacket>(), FHktStructSerializer::SerializeStructToBytes(Packet));
    }
    void ExecuteBehavior(int64 SubjectId, int32 BehaviorTypeId, const TArray<uint8>& Bytes);

    FOnSyncBehavior OnCreatedBehavior;
    FOnSyncBehavior OnDestroyedBehavior;

private:
    TSharedPtr<FHktRpcProxy> RpcProxy;
    TSharedPtr<FHktGraph> Graph;
};
