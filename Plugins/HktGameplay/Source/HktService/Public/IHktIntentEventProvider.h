#pragma once

#include "HktServiceInterfaces.h"
#include "IHktIntentEventProvider.generated.h"

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
    /** 외부 시스템(뷰, 로직 등)에서 호출하여 변경된 이벤트 히스토리를 가져가고 내부 버퍼를 비웁니다. */
    virtual bool FlushEvents(int32 ChannelId, int32& OutSyncedFrame, TArray<FHktIntentHistoryEntry>& OutHistory) = 0;
    
    // 현재 동기화된 서버 프레임을 반환합니다. (외부 조회용)
    virtual int32 GetCurrentServerFrame() const = 0;
};
