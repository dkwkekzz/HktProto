#include "HktIntentGameMode.h"
#include "HktIntentGameState.h"
#include "HktIntentPlayerState.h"
#include "HktIntentEventSubsystem.h"
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
		if (UHktIntentEventSubsystem* IntentEventSubsystem = UHktIntentEventSubsystem::Get(GetWorld()))
		{
			IntentEventSubsystem->PushIntentBatch(AbsoluteFrame);
		}
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
