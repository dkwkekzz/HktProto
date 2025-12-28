// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktVFXSubsystem.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void UHktVFXSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UHktVFXSubsystem::Deinitialize()
{
	LoadedSystems.Empty();
	RuntimeMappings.Empty();
	Super::Deinitialize();
}

bool UHktVFXSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld();
}

UNiagaraComponent* UHktVFXSubsystem::SpawnVFXAtLocation(
	FGameplayTag VFXTag,
	const FVector& Location,
	const FRotator& Rotation,
	FVector Scale)
{
	UNiagaraSystem* System = GetSystemForTag(VFXTag);
	if (!System)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Get mapping for scale override
	const FHktVFXMapping* Mapping = GetMappingForTag(VFXTag);
	if (Mapping && Scale == FVector(1.0f))
	{
		Scale = Mapping->Scale;
	}

	// Spawn Niagara system
	UNiagaraComponent* Component = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World,
		System,
		Location,
		Rotation,
		Scale,
		true,  // bAutoDestroy
		true,  // bAutoActivate
		ENCPoolMethod::None,
		true   // bPreCullCheck
	);

	// Play sound if configured
	if (Mapping && !Mapping->Sound.IsNull())
	{
		if (USoundBase* Sound = Mapping->Sound.LoadSynchronous())
		{
			UGameplayStatics::PlaySoundAtLocation(World, Sound, Location, Rotation);
		}
	}

	return Component;
}

UNiagaraComponent* UHktVFXSubsystem::SpawnVFXAttached(
	FGameplayTag VFXTag,
	USceneComponent* AttachComponent,
	FName SocketName,
	FVector LocationOffset)
{
	if (!AttachComponent)
	{
		return nullptr;
	}

	UNiagaraSystem* System = GetSystemForTag(VFXTag);
	if (!System)
	{
		return nullptr;
	}

	return UNiagaraFunctionLibrary::SpawnSystemAttached(
		System,
		AttachComponent,
		SocketName,
		LocationOffset,
		FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget,
		true,  // bAutoDestroy
		true,  // bAutoActivate
		ENCPoolMethod::None,
		true   // bPreCullCheck
	);
}

void UHktVFXSubsystem::PlayVFXOneShot(
	FGameplayTag VFXTag,
	const FVector& Location,
	const FRotator& Rotation)
{
	SpawnVFXAtLocation(VFXTag, Location, Rotation, FVector(1.0f));
}

void UHktVFXSubsystem::SetVFXConfig(UHktVFXConfig* Config)
{
	VFXConfig = Config;
	
	// Clear cached systems to reload with new config
	LoadedSystems.Empty();
}

void UHktVFXSubsystem::RegisterVFXMapping(const FHktVFXMapping& Mapping)
{
	// Check for existing mapping with same tag
	for (FHktVFXMapping& Existing : RuntimeMappings)
	{
		if (Existing.VFXTag == Mapping.VFXTag)
		{
			Existing = Mapping;
			LoadedSystems.Remove(Mapping.VFXTag);
			return;
		}
	}

	RuntimeMappings.Add(Mapping);
}

UNiagaraSystem* UHktVFXSubsystem::GetSystemForTag(const FGameplayTag& Tag)
{
	// Check cache first
	if (TObjectPtr<UNiagaraSystem>* Found = LoadedSystems.Find(Tag))
	{
		return *Found;
	}

	// Get mapping
	const FHktVFXMapping* Mapping = GetMappingForTag(Tag);
	if (!Mapping || Mapping->NiagaraSystem.IsNull())
	{
		return nullptr;
	}

	// Load the system
	UNiagaraSystem* System = Mapping->NiagaraSystem.LoadSynchronous();
	if (System)
	{
		LoadedSystems.Add(Tag, System);
	}

	return System;
}

const FHktVFXMapping* UHktVFXSubsystem::GetMappingForTag(const FGameplayTag& Tag) const
{
	// Check runtime mappings first (higher priority)
	for (const FHktVFXMapping& Mapping : RuntimeMappings)
	{
		if (Mapping.VFXTag.MatchesTag(Tag))
		{
			return &Mapping;
		}
	}

	// Check config mappings
	if (VFXConfig)
	{
		return VFXConfig->GetMappingForTag(Tag);
	}

	return nullptr;
}

