#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "HktStructSerializer.h"
#include "HktBehaviorFactory.h"
#include "HktClientCoreSubsystem.generated.h"

class IHktBehavior;
class FHktReliableUdpClient;
class FHktGraph;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBehaviorCreated, const IHktBehavior&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnBehaviorDestroyed, const IHktBehavior&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSubjectDestroyed, FHktId);

UCLASS()
class HKTCLIENT_API UHktClientCoreSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
    GENERATED_BODY()
    
public:
    static UHktClientCoreSubsystem* Get(UWorld* InWorld);

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
	// FTickableGameObject interface
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual ETickableTickType GetTickableTickType() const override { return (HasAnyFlags(RF_ClassDefaultObject) ? ETickableTickType::Never : ETickableTickType::Conditional); }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UTargetingSubsystem, STATGROUP_Tickables); }
	virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }
	// ~FTickableGameObject interface

    void Connect();
	void Disconnect();

    template<typename TFlagment>
    FORCEINLINE void SendFlagment(const TFlagment& InFlagment)
    {
        const FHktBehaviorRequestHeader Header = FHktBehaviorFactory::CreateBehaviorRequest<TFlagment>(MySubjectId, DefaultSyncGroupId, InFlagment);
        SendBytes(FHktStructSerializer::SerializeStructToBytes(Header));
    }
    void SendBytes(const TArray<uint8>& Bytes);

    FOnBehaviorCreated OnBehaviorCreated;
    FOnBehaviorDestroyed OnBehaviorDestroyed;
    FOnSubjectDestroyed OnSubjectDestroyed;

private:
    TSharedPtr<FHktReliableUdpClient> NetClient;
    TSharedPtr<FHktGraph> Graph;

    int64 MySubjectId = 1;
    constexpr static int64 DefaultSyncGroupId = 0;
};
