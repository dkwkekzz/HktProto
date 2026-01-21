#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HktIntentGameMode.generated.h"

class AHktIntentPlayerState;

/**
 * GameMode for HktIntent system.
 * Authoritative source of the deterministic frame number and channel.
 * 
 * Late Join 처리:
 * - PostLogin에서 플레이어 등록 및 스냅샷 전송
 */
UCLASS()
class HKTINTENT_API AHktIntentGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHktIntentGameMode();

	virtual void Tick(float DeltaSeconds) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	/** Get the absolute server frame */
	int32 GetServerFrame() const { return AbsoluteFrame; }

	/** Get the current channel ID (동기화 그룹 식별자) */
	int32 GetChannelId() const { return ChannelId; }

private:
	/** 플레이어에게 현재 Simulation 상태 스냅샷 전송 */
	void SendSnapshotToPlayer(AHktIntentPlayerState* PlayerState);

private:
	int32 AbsoluteFrame;
	float FrameAccumulator;

	/** Channel ID - 함께 동기화가 필요한 묶음 식별자 */
	int32 ChannelId;

	static constexpr float FixedFrameTime = 1.0f / 30.0f;
};
