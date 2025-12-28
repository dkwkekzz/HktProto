// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MassEntityTypes.h"
#include "Engine/DataAsset.h"
#include "HktAbilityTypes.generated.h"

class UNiagaraSystem;
class UAnimMontage;
class UMassEntityConfigAsset;

//-----------------------------------------------------------------------------
// Enums
//-----------------------------------------------------------------------------

UENUM(BlueprintType)
enum class EHktAbilityType : uint8
{
	None,
	Movement,
	MeleeAttack,
	Projectile,
	AreaOfEffect,
	Summon,
	Buff,
};

UENUM(BlueprintType)
enum class EHktDamageType : uint8
{
	Physical,
	Fire,
	Ice,
	Lightning,
	Holy,
	Dark,
};

UENUM(BlueprintType)
enum class EHktCombatState : uint8
{
	Idle,
	Moving,
	Attacking,
	CastingAbility,
	Stunned,
	Dead,
};

//-----------------------------------------------------------------------------
// Base Ability Data Asset
//-----------------------------------------------------------------------------

/**
 * Base data asset for ability configuration.
 * All ability types inherit from this.
 */
UCLASS(BlueprintType)
class HKTSIMULATION_API UHktAbilityDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Unique identifier tag for this ability */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	FGameplayTag AbilityTag;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	FText DisplayName;

	/** Ability type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	EHktAbilityType AbilityType = EHktAbilityType::None;

	/** Cooldown in frames (60fps) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	int32 CooldownFrames = 60;

	/** Cast time in frames before the ability activates */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	int32 CastTimeFrames = 0;

	/** Animation state tag to trigger */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	FGameplayTag AnimationTag;

	/** VFX to spawn on cast */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> CastVFX;

	/** Sound to play on cast */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TSoftObjectPtr<USoundBase> CastSound;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("HktAbility"), GetFName());
	}
};

//-----------------------------------------------------------------------------
// Melee Attack Data
//-----------------------------------------------------------------------------

/**
 * Configuration for melee attacks (sword, axe, etc.)
 */
UCLASS(BlueprintType)
class HKTSIMULATION_API UHktMeleeAbilityData : public UHktAbilityDataAsset
{
	GENERATED_BODY()

public:
	UHktMeleeAbilityData()
	{
		AbilityType = EHktAbilityType::MeleeAttack;
	}

	/** Base damage */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	float BaseDamage = 10.0f;

	/** Damage type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	EHktDamageType DamageType = EHktDamageType::Physical;

	/** Attack range */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float AttackRange = 150.0f;

	/** Attack arc in degrees (cone attack) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float AttackArc = 90.0f;

	/** Frame when damage is applied (during animation) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	int32 DamageFrameOffset = 20;

	/** Total attack duration in frames */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	int32 AttackDurationFrames = 40;

	/** VFX for hit impact */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> HitVFX;

	/** VFX for weapon swing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> SwingVFX;
};

//-----------------------------------------------------------------------------
// Projectile Data
//-----------------------------------------------------------------------------

/**
 * Configuration for projectile abilities (fireball, arrow, etc.)
 */
UCLASS(BlueprintType)
class HKTSIMULATION_API UHktProjectileAbilityData : public UHktAbilityDataAsset
{
	GENERATED_BODY()

public:
	UHktProjectileAbilityData()
	{
		AbilityType = EHktAbilityType::Projectile;
	}

	/** Projectile speed (units per second) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float ProjectileSpeed = 2000.0f;

	/** Projectile lifetime in frames */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	int32 LifetimeFrames = 300;

	/** Projectile radius for collision */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float ProjectileRadius = 30.0f;

	/** Does it explode on impact? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Explosion")
	bool bExplodesOnImpact = true;

	/** Explosion radius */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Explosion")
	float ExplosionRadius = 300.0f;

	/** Base damage */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	float BaseDamage = 25.0f;

	/** Damage type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	EHktDamageType DamageType = EHktDamageType::Fire;

	/** Projectile mesh/VFX */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> ProjectileVFX;

	/** Explosion VFX */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> ExplosionVFX;

	/** Explosion sound */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TSoftObjectPtr<USoundBase> ExplosionSound;
};

//-----------------------------------------------------------------------------
// Summon Data
//-----------------------------------------------------------------------------

/**
 * Configuration for summon abilities
 */
UCLASS(BlueprintType)
class HKTSIMULATION_API UHktSummonAbilityData : public UHktAbilityDataAsset
{
	GENERATED_BODY()

public:
	UHktSummonAbilityData()
	{
		AbilityType = EHktAbilityType::Summon;
	}

	/** Entity config to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Summon")
	TSoftObjectPtr<UMassEntityConfigAsset> SummonEntityConfig;

	/** Number of units to summon */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Summon")
	int32 SummonCount = 1;

	/** Spawn radius around target location */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Summon")
	float SpawnRadius = 100.0f;

	/** Summon lifetime in frames (0 = permanent) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Summon")
	int32 SummonLifetimeFrames = 0;

	/** Summon VFX */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> SummonVFX;
};

//-----------------------------------------------------------------------------
// Unit Data Asset
//-----------------------------------------------------------------------------

/**
 * Configuration for unit types (player, enemy, summon, etc.)
 */
UCLASS(BlueprintType)
class HKTSIMULATION_API UHktUnitDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Unit type tag */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit")
	FGameplayTag UnitTag;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit")
	FText DisplayName;

	/** Maximum health */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float MaxHealth = 100.0f;

	/** Movement speed (units per second) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float MoveSpeed = 400.0f;

	/** Attack power multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float AttackPower = 1.0f;

	/** Defense (damage reduction) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float Defense = 0.0f;

	/** Default abilities this unit has */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<TSoftObjectPtr<UHktAbilityDataAsset>> DefaultAbilities;

	/** Mesh for visualization */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TSoftObjectPtr<UStaticMesh> UnitMesh;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("HktUnit"), GetFName());
	}
};

//-----------------------------------------------------------------------------
// Runtime Fragments
//-----------------------------------------------------------------------------

/**
 * Fragment for entity health
 */
USTRUCT(BlueprintType)
struct HKTSIMULATION_API FHktHealthFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	float CurrentHealth = 100.0f;

	UPROPERTY()
	float MaxHealth = 100.0f;

	UPROPERTY()
	bool bIsDead = false;

	void TakeDamage(float Damage)
	{
		CurrentHealth = FMath::Max(0.0f, CurrentHealth - Damage);
		if (CurrentHealth <= 0.0f)
		{
			bIsDead = true;
		}
	}

	void Heal(float Amount)
	{
		if (!bIsDead)
		{
			CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + Amount);
		}
	}

	float GetHealthPercent() const
	{
		return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
	}
};

/**
 * Fragment for movement state
 */
USTRUCT(BlueprintType)
struct HKTSIMULATION_API FHktMovementFragment : public FMassFragment
{
	GENERATED_BODY()

	/** Target position to move to */
	UPROPERTY()
	FVector TargetPosition = FVector::ZeroVector;

	/** Current velocity */
	UPROPERTY()
	FVector Velocity = FVector::ZeroVector;

	/** Movement speed */
	UPROPERTY()
	float MoveSpeed = 400.0f;

	/** Is currently moving? */
	UPROPERTY()
	bool bIsMoving = false;

	/** Has reached destination? */
	UPROPERTY()
	bool bHasReachedTarget = false;
};

/**
 * Fragment for combat state
 */
USTRUCT(BlueprintType)
struct HKTSIMULATION_API FHktCombatFragment : public FMassFragment
{
	GENERATED_BODY()

	/** Current combat state */
	UPROPERTY()
	EHktCombatState CombatState = EHktCombatState::Idle;

	/** Current ability being used */
	UPROPERTY()
	FGameplayTag CurrentAbilityTag;

	/** Frame when current ability started */
	UPROPERTY()
	uint32 AbilityStartFrame = 0;

	/** Target entity for attack */
	UPROPERTY()
	FMassEntityHandle AttackTarget;

	/** Target location for abilities */
	UPROPERTY()
	FVector AbilityTargetLocation = FVector::ZeroVector;

	/** Attack power multiplier */
	UPROPERTY()
	float AttackPower = 1.0f;

	/** Defense value */
	UPROPERTY()
	float Defense = 0.0f;

	/** Team ID for friend/foe detection */
	UPROPERTY()
	int32 TeamId = 0;

	bool IsInCombatAction() const
	{
		return CombatState == EHktCombatState::Attacking || CombatState == EHktCombatState::CastingAbility;
	}
};

/**
 * Fragment for projectiles
 */
USTRUCT(BlueprintType)
struct HKTSIMULATION_API FHktProjectileFragment : public FMassFragment
{
	GENERATED_BODY()

	/** Owner entity that fired this */
	UPROPERTY()
	FMassEntityHandle OwnerEntity;

	/** Owner's team ID */
	UPROPERTY()
	int32 OwnerTeamId = 0;

	/** Projectile ability data reference */
	UPROPERTY()
	FGameplayTag AbilityTag;

	/** Target position */
	UPROPERTY()
	FVector TargetPosition = FVector::ZeroVector;

	/** Current velocity */
	UPROPERTY()
	FVector Velocity = FVector::ZeroVector;

	/** Speed */
	UPROPERTY()
	float Speed = 2000.0f;

	/** Damage to deal */
	UPROPERTY()
	float Damage = 25.0f;

	/** Explosion radius */
	UPROPERTY()
	float ExplosionRadius = 300.0f;

	/** Frame when spawned */
	UPROPERTY()
	uint32 SpawnFrame = 0;

	/** Lifetime in frames */
	UPROPERTY()
	int32 LifetimeFrames = 300;

	/** Has hit something? */
	UPROPERTY()
	bool bHasHit = false;

	/** Should explode on impact? */
	UPROPERTY()
	bool bExplodesOnImpact = true;
};

/**
 * Fragment for summoned units
 */
USTRUCT(BlueprintType)
struct HKTSIMULATION_API FHktSummonFragment : public FMassFragment
{
	GENERATED_BODY()

	/** Owner entity that summoned this */
	UPROPERTY()
	FMassEntityHandle OwnerEntity;

	/** Frame when summoned */
	UPROPERTY()
	uint32 SummonFrame = 0;

	/** Lifetime in frames (0 = permanent) */
	UPROPERTY()
	int32 LifetimeFrames = 0;

	/** Is this a summoned unit? */
	UPROPERTY()
	bool bIsSummoned = false;
};

/**
 * Tag to identify projectile entities
 */
USTRUCT(BlueprintType)
struct HKTSIMULATION_API FHktProjectileTag : public FMassTag
{
	GENERATED_BODY()
};

/**
 * Tag to identify player-controllable entities
 */
USTRUCT(BlueprintType)
struct HKTSIMULATION_API FHktPlayerControlledTag : public FMassTag
{
	GENERATED_BODY()
};

/**
 * Tag to identify selectable entities
 */
USTRUCT(BlueprintType)
struct HKTSIMULATION_API FHktSelectableTag : public FMassTag
{
	GENERATED_BODY()
};

