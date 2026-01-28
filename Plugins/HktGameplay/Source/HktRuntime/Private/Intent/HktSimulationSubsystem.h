#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HktCoreTypes.h"
#include "HktVMProcessor.h"
#include "HktSimulationSubsystem.generated.h"

/**
 * [UHktSimulationSubsystem]
 * 
 * 의도 이벤트 중앙 관리자
 */
UCLASS()
class HKTRUNTIME_API UHktSimulationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
	static UHktSimulationSubsystem* Get(const UObject* WorldContextObject);

    void Execute(const FHktIntentEvent& Event);

private:
    FHktVMProcessor VMProcessor;
};
