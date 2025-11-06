// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktProtoCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "HktStateTreeComponent.h"

AHktProtoCharacter::AHktProtoCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// 1. 컴포넌트 생성
	StateTreeComponent = CreateDefaultSubobject<UHktStateTreeComponent>(TEXT("StateTreeComponent"));

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AHktProtoCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 2. [핵심] 컴포넌트의 델리게이트에 내 함수(OnStateTagChanged)를 바인딩
	if (StateTreeComponent)
	{
		StateTreeComponent->OnStateTagChanged.AddUObject(this, &AHktProtoCharacter::OnStateTagChanged);
	}
}

void AHktProtoCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void AHktProtoCharacter::OnStateTagChanged(FGameplayTag Tag, bool bIsAdded)
{
	UE_LOG(LogTemp, Log, TEXT("OnStateTagChanged: %s, %d"), *Tag.ToString(), bIsAdded);
}