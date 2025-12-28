#include "HktIntentGameMode.h"
#include "HktIntentGameState.h"

AHktIntentGameMode::AHktIntentGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	GameStateClass = AHktIntentGameState::StaticClass();
	
	AbsoluteFrame = 0;
	FrameAccumulator = 0.0f;
}

void AHktIntentGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Accumulate time and advance frames
	FrameAccumulator += DeltaSeconds;
	while (FrameAccumulator >= FixedFrameTime)
	{
		FrameAccumulator -= FixedFrameTime;
		AbsoluteFrame++;
	}

	// Update GameState for replication
	if (AHktIntentGameState* HktGameState = GetGameState<AHktIntentGameState>())
	{
		HktGameState->SetServerFrame(AbsoluteFrame);
	}
}
