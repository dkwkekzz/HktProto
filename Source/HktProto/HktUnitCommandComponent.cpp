// Fill out your copyright notice in the Description page of Project Settings.

#include "HktUnitCommandComponent.h"
#include "HktRtsUnit.h"
#include "GameFramework/PlayerController.h"

UHktUnitCommandComponent::UHktUnitCommandComponent()
{
	SetIsReplicatedByDefault(true);
}

void UHktUnitCommandComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UHktUnitCommandComponent::RequestMoveUnits(const TArray<TWeakObjectPtr<AHktRtsUnit>>& InSelectedUnits, const FVector& TargetLocation)
{
	// TWeakObjectPtr to APawn*
	TArray<APawn*> SelectedPawns;
	for (const auto& Unit : InSelectedUnits)
	{
		if (Unit.IsValid())
		{
			SelectedPawns.Add(Cast<APawn>(Unit.Get()));
		}
	}
	
	Server_RequestMoveUnits(SelectedPawns, TargetLocation);
}

bool UHktUnitCommandComponent::Server_RequestMoveUnits_Validate(const TArray<APawn*>& InSelectedUnits, const FVector& TargetLocation)
{
	// TODO: Add more robust validation, e.g., check distance, cooldowns, etc.
	return true;
}

void UHktUnitCommandComponent::Server_RequestMoveUnits_Implementation(const TArray<APawn*>& InSelectedUnits, const FVector& TargetLocation)
{
	// This code only runs on the server.
	for (APawn* UnitPawn : InSelectedUnits)
	{
		if (ValidateUnitCommand(UnitPawn, TargetLocation))
		{
			if (AHktRtsUnit* Unit = Cast<AHktRtsUnit>(UnitPawn))
			{
				Unit->MoveUnitTo(TargetLocation);
			}
		}
	}
}

bool UHktUnitCommandComponent::ValidateUnitCommand(APawn* Unit, const FVector& Location)
{
	if (!Unit)
	{
		return false;
	}

	// Check if the commanding player owns this unit.
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC || Unit->GetOwner() != PC)
	{
		// This check might be too simple. Depending on the game, ownership might be on the PlayerState or a team manager.
		// return false; 
	}
	
	return true;
}
