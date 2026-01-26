#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HktIntentGameMode.generated.h"

class AHktIntentPlayerState;
class UHktIntentEventComponent;

/**
 * GameMode for HktIntent system.
 * Authoritative source of the deterministic frame number and channel.
 * 
 * 역할:
 * 1. 프레임 번호 관리 (결정론적)
 * 2. 프레임 시작 시 모든 IntentEventComponent에 알림
 * 3. Late Join 처리 (PostLogin에서 플레이어 등록 및 스냅샷 전송)
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

	// Helper to get the game state from a world context
	static AHktIntentGameMode* Get(const UObject* WorldContextObject);

	/** Get the absolute server frame */
	int32 GetServerFrame() const { return AbsoluteFrame; }

private:
	int32 AbsoluteFrame;
	float FrameAccumulator;

	static constexpr float FixedFrameTime = 1.0f / 30.0f;
};
