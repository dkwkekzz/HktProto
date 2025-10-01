#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HktDef.h"
#include "HktStructSerializer.h"
#include "HktClientCoreSubsystem.generated.h"

class IHktBehavior;
class FHktRpcProxy;
class FHktGraph;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBehaviorCreated, const IHktBehavior&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnBehaviorDestroyed, const IHktBehavior&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSubjectDestroyed, FHktId);

UCLASS()
class HKTCLIENT_API UHktClientCoreSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
    
public:
    static UHktClientCoreSubsystem* Get(UWorld* InWorld);

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void SyncGroup(int64 PlayerId, int64 GroupId);

    template<typename TPacket>
    FORCEINLINE void ExecuteBehavior(int64 SubjectId, const TPacket& Packet) 
    { 
        ExecuteBehavior(SubjectId, GetBehaviorTypeId<TPacket>(), FHktStructSerializer::SerializeStructToBytes(Packet));
    }
    void ExecuteBehavior(int64 SubjectId, int32 BehaviorTypeId, const TArray<uint8>& Bytes);

    FOnBehaviorCreated OnBehaviorCreated;
    FOnBehaviorDestroyed OnBehaviorDestroyed;
    FOnSubjectDestroyed OnSubjectDestroyed;

private:
    TSharedPtr<FHktRpcProxy> RpcProxy;
    TSharedPtr<FHktGraph> Graph;
};
