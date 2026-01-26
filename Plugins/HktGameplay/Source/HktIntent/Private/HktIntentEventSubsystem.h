#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HktIntentInterface.h"
#include "HktIntentEventSubsystem.generated.h"

/**
 * [UHktIntentEventSubsystem]
 * 
 * 의도 이벤트 중앙 관리자
 */
UCLASS()
class HKTINTENT_API UHktIntentEventSubsystem : public UWorldSubsystem, public IHktIntentEventProvider
{
	GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
	static UHktIntentEventSubsystem* Get(const UObject* WorldContextObject);

    void RegisterIntentEventComponent(UHktIntentEventComponent* Component);
    void UnregisterIntentEventComponent(UHktIntentEventComponent* Component);

    void PushIntentBatch(int32 FrameNumber);

    // IHktIntentEventProvider Interface
    virtual void PullIntentEvents(int32 CompletedFrameNumber, TArray<FHktIntentEvent>& OutIntentEvents) override;
    // End IHktIntentEventProvider Interface

private:
    /** * 등록된 IntentEventComponent 목록 */
    UPROPERTY()
    TArray<TObjectPtr<UHktIntentEventComponent>> IntentEventComponents;
};
