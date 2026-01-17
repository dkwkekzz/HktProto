#include "HktIntentPlayerState.h"
#include "HktIntentEventComponent.h"
#include "HktIntentBuilderComponent.h"

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
