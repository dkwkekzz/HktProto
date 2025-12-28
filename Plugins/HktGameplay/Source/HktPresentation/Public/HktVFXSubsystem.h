// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "HktVFXSubsystem.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class USoundBase;

/**
 * VFX mapping from gameplay tag to Niagara system.
 */
USTRUCT(BlueprintType)
struct HKTPRESENTATION_API FHktVFXMapping
{
	GENERATED_BODY()

	/** The gameplay tag that triggers this VFX */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FGameplayTag VFXTag;

	/** Niagara system to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

	/** Optional sound to play */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<USoundBase> Sound;

	/** Default scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FVector Scale = FVector(1.0f);

	/** Auto-destroy after completion? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	bool bAutoDestroy = true;

	/** Pooled for reuse? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	bool bPooled = false;
};

/**
 * VFX configuration data asset.
 */
UCLASS(BlueprintType)
class HKTPRESENTATION_API UHktVFXConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** All VFX mappings */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TArray<FHktVFXMapping> VFXMappings;

	/** Get mapping for a tag */
	const FHktVFXMapping* GetMappingForTag(const FGameplayTag& Tag) const
	{
		for (const FHktVFXMapping& Mapping : VFXMappings)
		{
			if (Mapping.VFXTag.MatchesTag(Tag))
			{
				return &Mapping;
			}
		}
		return nullptr;
	}
};

/**
 * Subsystem for managing visual effects.
 * 
 * Maps gameplay tags to Niagara systems and handles VFX lifecycle.
 * Used by HktPresentation to render effects based on simulation output.
 */
UCLASS()
class HKTPRESENTATION_API UHktVFXSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin UWorldSubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End UWorldSubsystem Interface

	/**
	 * Spawn VFX at a location using a gameplay tag.
	 * @param VFXTag Tag identifying which VFX to spawn
	 * @param Location World location
	 * @param Rotation World rotation
	 * @param Scale Optional scale override
	 * @return Spawned Niagara component (nullptr if tag not found)
	 */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	UNiagaraComponent* SpawnVFXAtLocation(
		FGameplayTag VFXTag,
		const FVector& Location,
		const FRotator& Rotation = FRotator::ZeroRotator,
		FVector Scale = FVector(1.0f)
	);

	/**
	 * Spawn VFX attached to a component.
	 * @param VFXTag Tag identifying which VFX to spawn
	 * @param AttachComponent Component to attach to
	 * @param SocketName Optional socket name
	 * @param LocationOffset Optional location offset
	 * @return Spawned Niagara component
	 */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	UNiagaraComponent* SpawnVFXAttached(
		FGameplayTag VFXTag,
		USceneComponent* AttachComponent,
		FName SocketName = NAME_None,
		FVector LocationOffset = FVector::ZeroVector
	);

	/**
	 * Play a one-shot VFX (fire and forget).
	 * @param VFXTag Tag identifying which VFX to play
	 * @param Location World location
	 * @param Rotation World rotation
	 */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void PlayVFXOneShot(
		FGameplayTag VFXTag,
		const FVector& Location,
		const FRotator& Rotation = FRotator::ZeroRotator
	);

	/**
	 * Set the VFX configuration.
	 * @param Config VFX config data asset
	 */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SetVFXConfig(UHktVFXConfig* Config);

	/**
	 * Register a VFX mapping at runtime.
	 * @param Mapping The VFX mapping to register
	 */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void RegisterVFXMapping(const FHktVFXMapping& Mapping);

private:
	/** Current VFX configuration */
	UPROPERTY()
	TObjectPtr<UHktVFXConfig> VFXConfig;

	/** Runtime-registered mappings */
	UPROPERTY()
	TArray<FHktVFXMapping> RuntimeMappings;

	/** Cached/pooled Niagara systems (loaded) */
	TMap<FGameplayTag, TObjectPtr<UNiagaraSystem>> LoadedSystems;

	/** Get Niagara system for tag (loads if needed) */
	UNiagaraSystem* GetSystemForTag(const FGameplayTag& Tag);

	/** Get mapping for tag (from config or runtime) */
	const FHktVFXMapping* GetMappingForTag(const FGameplayTag& Tag) const;
};

