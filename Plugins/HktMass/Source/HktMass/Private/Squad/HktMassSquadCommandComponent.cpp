#include "HktMassSquadCommandComponent.h"
#include "HktMassSquadSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

UHktMassSquadCommandComponent::UHktMassSquadCommandComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;
}

void UHktMassSquadCommandComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHktMassSquadCommandComponent, SquadID);
	DOREPLIFETIME(UHktMassSquadCommandComponent, SquadLocation);
}

void UHktMassSquadCommandComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UHktMassSquadSubsystem* Subsystem = UWorld::GetSubsystem<UHktMassSquadSubsystem>(GetWorld()))
	{
		Subsystem->RegisterSquadCommandComponent(SquadID, this);
	}
}

void UHktMassSquadCommandComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UHktMassSquadSubsystem* Subsystem = UWorld::GetSubsystem<UHktMassSquadSubsystem>(GetWorld()))
	{
		Subsystem->UnregisterSquadCommandComponent(SquadID);
	}

	Super::EndPlay(EndPlayReason);
}

void UHktMassSquadCommandComponent::UpdateSquadLocation(const FVector& NewLocation)
{
	SquadLocation = NewLocation;
}

