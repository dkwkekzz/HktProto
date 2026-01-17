#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HktIntentInterfaces.h"
#include "IHktIntentEventProvider.h"
#include "HktIntentSubsystem.generated.h"

// ------------------------------------------------------------------------------------------------
// [Subsystem Implementation]
// ------------------------------------------------------------------------------------------------

UCLASS()
class UHktIntentSubsystem : public UWorldSubsystem, public IHktIntentEventProvider
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Helper to get the subsystem from a world context
    static UHktIntentSubsystem* Get(UWorld* World);

    // --- UHktIntentEventProvider 인터페이스 구현 ---
    virtual TSharedRef<IHktIntentChannel> CreateOrGetChannel(int32 InChannelId) override;
    virtual TSharedPtr<IHktIntentChannel> GetChannel(int32 InChannelId) override;

private:
    UPROPERTY(Transient)
    TMap<int32, TSharedRef<IHktIntentChannel>> ChannelMap;
};
