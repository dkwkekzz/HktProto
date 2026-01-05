#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "HktIntentInterfaces.h"
#include "IHktIntentEventProvider.h"
#include "HktIntentSubsystem.generated.h"

// 채널별로 관리되는 이벤트 상태 데이터
struct FHktChannelIntentData
{
    // 현재 활성화된 인텐트 목록 (State)
    TArray<FHktIntentEvent> ActiveIntents;

    // 외부로 제공되기 위해 대기 중인 이벤트 히스토리 (Buffer)
    // 발생 순서대로 추가/제거 이력을 기록
    TArray<FHktIntentHistoryEntry> HistoryBuffer;

    // 이 채널이 마지막으로 동기화된 서버 프레임
    int32 LastSyncedFrame = -1;
};

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

    // --- UHktIntentComponent 인터페이스 ---
    /** 인텐트 추가: Subject와 Tag가 겹치지 않는다고 가정하거나, 중복 허용 정책에 따름 */
    void AddIntentEvent(int32 ChannelId, const FHktIntentEvent& InEvent);

    /** 인텐트 제거: Subject와 Tag가 일치하는 이벤트를 찾아 제거 */
    void RemoveIntentEvent(int32 ChannelId, const FHktIntentEvent& InEvent);

    /** 인텐트 갱신: 기존 인텐트를 찾아 Target이나 Frame 등을 변경 (Remove -> Add 로직으로 처리하여 변경 사항 전파) */
    void UpdateIntentEvent(int32 ChannelId, const FHktIntentEvent& InNewEvent);

    /** Set the current server frame (Called by GameMode) */
    void SetCurrentServerFrame(int32 FrameNumber);

    // --- UHktIntentEventProvider 인터페이스 구현 ---

    /** 외부 시스템(뷰, 로직 등)에서 호출하여 변경된 이벤트 히스토리를 가져가고 내부 버퍼를 비웁니다. */
    virtual bool FlushEvents(int32 ChannelId, int32& OutSyncedFrame, TArray<FHktIntentHistoryEntry>& OutHistory) override;

    /** 현재 동기화된 서버 프레임을 반환합니다. (외부 조회용) */
    virtual int32 GetCurrentServerFrame() const override;

private:
    FHktChannelIntentData& GetChannelData(int32 ChannelId);

private:
    // 채널 ID를 키로 하는 데이터 맵
    TMap<int32, FHktChannelIntentData> ChannelMap;

    // 현재 월드의 글로벌 서버 프레임 (동기화 기준)
    int32 CurrentServerFrame = 0;
};
