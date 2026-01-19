#include "HktIntentPlayerState.h"
#include "HktIntentEventComponent.h"
#include "HktIntentBuilderComponent.h"
#include "HktAttributeComponent.h"
#include "Net/UnrealNetwork.h"

//-----------------------------------------------------------------------------
// AHktIntentPlayerState
//-----------------------------------------------------------------------------

AHktIntentPlayerState::AHktIntentPlayerState()
{
	IntentEventComponent = CreateDefaultSubobject<UHktIntentEventComponent>(TEXT("IntentEventComponent"));
	AttributeComponent = CreateDefaultSubobject<UHktAttributeComponent>(TEXT("AttributeComponent"));
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
		
		// AttributeComponent에도 전달
		if (AttributeComponent)
		{
			AttributeComponent->SetPlayerHandle(InHandle);
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
