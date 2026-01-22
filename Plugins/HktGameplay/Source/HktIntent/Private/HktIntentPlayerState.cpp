#include "HktIntentPlayerState.h"
#include "HktIntentEventComponent.h"
#include "HktIntentBuilderComponent.h"
#include "HktIntentGameMode.h"
#include "HktSimulationStashComponent.h"
#include "HktServiceSubsystem.h"
#include "Net/UnrealNetwork.h"

//-----------------------------------------------------------------------------
// AHktIntentPlayerState
//-----------------------------------------------------------------------------

AHktIntentPlayerState::AHktIntentPlayerState()
{
	// Intent 전송 컴포넌트 생성
	IntentEventComponent = CreateDefaultSubobject<UHktIntentEventComponent>(TEXT("IntentEventComponent"));
	
	// 시뮬레이션 상태 관리 컴포넌트 생성
	SimulationStashComponent = CreateDefaultSubobject<UHktSimulationStashComponent>(TEXT("SimulationStashComponent"));
}

void AHktIntentPlayerState::BeginPlay()
{
	Super::BeginPlay();
	
	// IntentEventComponent와 StashComponent 연결
	if (IntentEventComponent && SimulationStashComponent)
	{
		IntentEventComponent->SetStashComponent(SimulationStashComponent);
	}
	
	// GameMode에 등록 (서버에서만)
	RegisterWithGameMode();
}

void AHktIntentPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// GameMode에서 등록 해제
	UnregisterFromGameMode();
	
	Super::EndPlay(EndPlayReason);
}

void AHktIntentPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AHktIntentPlayerState, PlayerHandle);
}

//-----------------------------------------------------------------------------
// GameMode Registration
//-----------------------------------------------------------------------------

void AHktIntentPlayerState::RegisterWithGameMode()
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (UWorld* World = GetWorld())
	{
		if (AHktIntentGameMode* GameMode = World->GetAuthGameMode<AHktIntentGameMode>())
		{
			if (IntentEventComponent)
			{
				GameMode->RegisterIntentEventComponent(IntentEventComponent);
			}
		}
	}
}

void AHktIntentPlayerState::UnregisterFromGameMode()
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (UWorld* World = GetWorld())
	{
		if (AHktIntentGameMode* GameMode = World->GetAuthGameMode<AHktIntentGameMode>())
		{
			if (IntentEventComponent)
			{
				GameMode->UnregisterIntentEventComponent(IntentEventComponent);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Player Handle
//-----------------------------------------------------------------------------

void AHktIntentPlayerState::SetPlayerHandle(const FHktPlayerHandle& InHandle)
{
	if (HasAuthority())
	{
		PlayerHandle = InHandle;
	}
}

//-----------------------------------------------------------------------------
// Late Join Snapshot
//-----------------------------------------------------------------------------

void AHktIntentPlayerState::Client_InitializeSnapshot_Implementation(const TArray<float>& AttributeValues)
{
	// Late Join: HktService를 통해 SimulationProvider에 스냅샷 전달
	if (UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld()))
	{
		if (TScriptInterface<IHktSimulationProvider> SimProvider = Service->GetSimulationProvider())
		{
			SimProvider->InitializePlayerFromSnapshot(PlayerHandle, AttributeValues);
			UE_LOG(LogTemp, Log, TEXT("[PlayerState] Late Join: Initialized player %d from server snapshot"), PlayerHandle.Value);
		}
	}
}

