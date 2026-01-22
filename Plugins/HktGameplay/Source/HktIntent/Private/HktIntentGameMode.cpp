#include "HktIntentGameMode.h"
#include "HktIntentGameState.h"
#include "HktIntentPlayerState.h"
#include "HktIntentEventComponent.h"
#include "HktServiceSubsystem.h"
#include "GameFramework/PlayerController.h"

AHktIntentGameMode::AHktIntentGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	GameStateClass = AHktIntentGameState::StaticClass();
	PlayerStateClass = AHktIntentPlayerState::StaticClass();
	
	AbsoluteFrame = 0;
	FrameAccumulator = 0.0f;
	ChannelId = 1; // Default channel
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
		
		// 프레임 시작 시 모든 IntentEventComponent에 알림
		NotifyFrameStartToAllComponents(AbsoluteFrame);
	}

	// Update GameState for replication
	if (AHktIntentGameState* HktGameState = GetGameState<AHktIntentGameState>())
	{
		HktGameState->SetServerFrame(AbsoluteFrame);
	}
}

void AHktIntentGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer)
	{
		return;
	}

	if (AHktIntentPlayerState* PlayerState = NewPlayer->GetPlayerState<AHktIntentPlayerState>())
	{
		FHktPlayerHandle Handle = PlayerState->GetPlayerHandle();

		UE_LOG(LogTemp, Log, TEXT("[GameMode] Logout: Unregistered player %d"), Handle.Value);
	}

	UE_LOG(LogTemp, Log, TEXT("[GameMode] PostLogin: Registered player %d"), Handle.Value);
}

void AHktIntentGameMode::Logout(AController* Exiting)
{
	if (APlayerController* PC = Cast<APlayerController>(Exiting))
	{
		if (AHktIntentPlayerState* PlayerState = PC->GetPlayerState<AHktIntentPlayerState>())
		{
			FHktPlayerHandle Handle = PlayerState->GetPlayerHandle();

			UE_LOG(LogTemp, Log, TEXT("[GameMode] Logout: Unregistered player %d"), Handle.Value);
		}
	}

	Super::Logout(Exiting);
}

AHktIntentGameMode* AHktIntentGameMode::Get(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return World->GetAuthGameMode<AHktIntentGameMode>();
	}
	return nullptr;
}

// ============================================================================
// IntentEventComponent Management
// ============================================================================

void AHktIntentGameMode::RegisterIntentEventComponent(UHktIntentEventComponent* Component)
{
	RegisteredEventComponents.Add(Component);
	
	UE_LOG(LogTemp, Log, TEXT("[GameMode] RegisterIntentEventComponent: Total=%d"), RegisteredEventComponents.Num());
}

void AHktIntentGameMode::UnregisterIntentEventComponent(UHktIntentEventComponent* Component)
{
	RegisteredEventComponents.Remove(Component);

	UE_LOG(LogTemp, Log, TEXT("[GameMode] UnregisterIntentEventComponent: Total=%d"), RegisteredEventComponents.Num());
}

void AHktIntentGameMode::NotifyFrameStartToAllComponents(int32 FrameNumber)
{
	for (UHktIntentEventComponent* Component : RegisteredEventComponents)
	{
		Component->NotifyIntentBatch(FrameNumber);
	}
}
