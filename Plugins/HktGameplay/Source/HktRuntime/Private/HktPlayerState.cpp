#include "HktPlayerState.h"

//-----------------------------------------------------------------------------
// AHktPlayerState
//-----------------------------------------------------------------------------

AHktIntentPlayerState::AHktIntentPlayerState()
{
}

void AHktIntentPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void AHktIntentPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AHktIntentPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
