#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HktClientCoreSubsystem.generated.h"

class FHktRpcProxy;

UCLASS()
class HKTCLIENT_API UHktClientCoreSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
    
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:
    TSharedPtr<FHktRpcProxy> RpcProxy;
};
