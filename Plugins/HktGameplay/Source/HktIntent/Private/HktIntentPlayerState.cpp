#include "HktIntentPlayerState.h"
#include "HktIntentEventComponent.h"
#include "HktIntentBuilderComponent.h"
#include "HktServiceSubsystem.h"
#include "Net/UnrealNetwork.h"

//-----------------------------------------------------------------------------
// AHktIntentPlayerState
//-----------------------------------------------------------------------------

AHktIntentPlayerState::AHktIntentPlayerState()
{
	IntentEventComponent = CreateDefaultSubobject<UHktIntentEventComponent>(TEXT("IntentEventComponent"));
}

void AHktIntentPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void AHktIntentPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AHktIntentPlayerState, PlayerHandle);
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

//-----------------------------------------------------------------------------
// Intent Submission
//-----------------------------------------------------------------------------

void AHktIntentPlayerState::SubmitIntent(UHktIntentBuilderComponent* Builder)
{
	if (IntentEventComponent)
	{
		IntentEventComponent->SubmitIntent(Builder);
	}
}
