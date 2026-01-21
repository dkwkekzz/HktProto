#include "HktIntentGameMode.h"
#include "HktIntentGameState.h"
#include "HktIntentPlayerState.h"
#include "HktIntentSubsystem.h"
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

	AHktIntentPlayerState* PlayerState = NewPlayer->GetPlayerState<AHktIntentPlayerState>();
	if (!PlayerState)
	{
		return;
	}

	// HktService를 통해 SimulationProvider 접근
	UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld());
	if (!Service)
	{
		return;
	}

	TScriptInterface<IHktSimulationProvider> SimProvider = Service->GetSimulationProvider();
	if (!SimProvider)
	{
		return;
	}

	// 1. 플레이어 등록
	FHktPlayerHandle Handle = SimProvider->RegisterPlayer();

	// 2. IntentSubsystem에 PlayerState 등록
	if (UHktIntentSubsystem* IntentSub = UHktIntentSubsystem::Get(GetWorld()))
	{
		IntentSub->RegisterPlayerState(PlayerState, Handle);
	}

	// 3. Late Join 클라이언트에게 스냅샷 전송
	SendSnapshotToPlayer(PlayerState);

	UE_LOG(LogTemp, Log, TEXT("[GameMode] PostLogin: Registered player %d"), Handle.Value);
}

void AHktIntentGameMode::Logout(AController* Exiting)
{
	if (APlayerController* PC = Cast<APlayerController>(Exiting))
	{
		if (AHktIntentPlayerState* PlayerState = PC->GetPlayerState<AHktIntentPlayerState>())
		{
			FHktPlayerHandle Handle = PlayerState->GetPlayerHandle();

			// IntentSubsystem에서 등록 해제
			if (UHktIntentSubsystem* IntentSub = UHktIntentSubsystem::Get(GetWorld()))
			{
				IntentSub->UnregisterPlayerState(PlayerState);
			}

			// HktService를 통해 SimulationProvider에서 등록 해제
			if (UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld()))
			{
				if (TScriptInterface<IHktSimulationProvider> SimProvider = Service->GetSimulationProvider())
				{
					SimProvider->UnregisterPlayer(Handle);
				}
			}

			UE_LOG(LogTemp, Log, TEXT("[GameMode] Logout: Unregistered player %d"), Handle.Value);
		}
	}

	Super::Logout(Exiting);
}

void AHktIntentGameMode::SendSnapshotToPlayer(AHktIntentPlayerState* PlayerState)
{
	if (!PlayerState)
	{
		return;
	}

	UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld());
	if (!Service)
	{
		return;
	}

	TScriptInterface<IHktSimulationProvider> SimProvider = Service->GetSimulationProvider();
	if (!SimProvider)
	{
		return;
	}

	FHktPlayerHandle Handle = PlayerState->GetPlayerHandle();
	if (!Handle.IsValid())
	{
		return;
	}

	// SimulationProvider를 통해 스냅샷 조회
	TArray<float> Values;
	if (!SimProvider->GetPlayerSnapshot(Handle, Values))
	{
		return;
	}

	// Client RPC로 스냅샷 전송
	PlayerState->Client_InitializeSnapshot(Values);

	UE_LOG(LogTemp, Log, TEXT("[GameMode] Sent snapshot to player %d (%d attributes)"), Handle.Value, Values.Num());
}
