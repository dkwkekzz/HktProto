#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HktIntentGameMode.generated.h"

/**
 * GameMode for HktIntent system.
 * Authoritative source of the deterministic frame number.
 */
UCLASS()
class HKTINTENT_API AHktIntentGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHktIntentGameMode();

	virtual void Tick(float DeltaSeconds) override;

	/** Get the absolute server frame */
	int32 GetServerFrame() const { return AbsoluteFrame; }

private:
	int32 AbsoluteFrame;
	float FrameAccumulator;

	static constexpr float FixedFrameTime = 1.0f / 30.0f;
};

