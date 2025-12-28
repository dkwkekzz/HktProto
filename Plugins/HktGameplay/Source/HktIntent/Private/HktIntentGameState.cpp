#include "HktIntentGameState.h"
#include "Net/UnrealNetwork.h"

AHktIntentGameState::AHktIntentGameState()
{
	PrimaryActorTick.bCanEverTick = true;
	ServerFrame = 0;
	ClientEstimatedFrame = 0;
	FrameAccumulator = 0.0f;
}

void AHktIntentGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHktIntentGameState, ServerFrame);
}

void AHktIntentGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetLocalRole() == ROLE_Authority)
	{
		// Server: Frame count is driven by GameMode (or manually set)
		// We just ensure ClientEstimatedFrame tracks it for server-side queries
		ClientEstimatedFrame = ServerFrame;
	}
	else
	{
		// Client: Predict frame advancement
		FrameAccumulator += DeltaSeconds;
		while (FrameAccumulator >= FixedFrameTime)
		{
			FrameAccumulator -= FixedFrameTime;
			ClientEstimatedFrame++;
		}
	}
}

void AHktIntentGameState::OnRep_ServerFrame()
{
	// Simple reconciliation: Snap to server frame if drift is too large
	int32 Diff = ClientEstimatedFrame - ServerFrame;
	if (FMath::Abs(Diff) > 5) 
	{
		ClientEstimatedFrame = ServerFrame;
	}
}

int32 AHktIntentGameState::GetCurrentFrame() const
{
	return ClientEstimatedFrame;
}
