#pragma once

#include "HktServiceInterfaces.h"
#include "IHktIntentEventProvider.generated.h"

class IHktIntentChannel;

UINTERFACE(MinimalAPI, BlueprintType)
class UHktIntentEventProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for a system that provides Intent Events to consumers (Simulation).
 * Implemented by HktIntent module.
 * 
 * Channel: 함께 동기화가 필요한 이벤트 그룹
 */
class HKTSERVICE_API IHktIntentEventProvider
{
	GENERATED_BODY()

public:
	virtual TSharedRef<IHktIntentChannel> CreateOrGetChannel(int32 InChannelId) = 0;
	virtual TSharedPtr<IHktIntentChannel> GetChannel(int32 InChannelId) = 0;
};
