// Fill out your copyright notice in the Description page of Project Settings.

#include "HktRtsPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "HktRtsUnit.h"
#include "HktRtsCameraPawn.h" // 카메라 폰 헤더
#include "HktUnitCommandComponent.h"
#include "HktConstructionComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

AHktRtsPlayerController::AHktRtsPlayerController()
{
	// 마우스 커서 표시
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	UnitCommandComponent = CreateDefaultSubobject<UHktUnitCommandComponent>(TEXT("UnitCommandComponent"));
	ConstructionComponent = CreateDefaultSubobject<UHktConstructionComponent>(TEXT("ConstructionComponent"));
}

void AHktRtsPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UnitCommandComponent)
	{
		UnitCommandComponent->RegisterComponentWithWorld(GetWorld());
	}

	if (ConstructionComponent)
	{
		ConstructionComponent->RegisterComponentWithWorld(GetWorld());
	}
}

void AHktRtsPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (LeftClickAction)
		{
			EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Triggered, this, &AHktRtsPlayerController::HandleLeftClick);
		}

		if (RightClickAction)
		{
			EnhancedInputComponent->BindAction(RightClickAction, ETriggerEvent::Triggered, this, &AHktRtsPlayerController::HandleRightClick);
		}

		if (ZoomAction)
		{
			EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &AHktRtsPlayerController::HandleZoom);
		}

		for (int32 CommandIndex = 0; CommandIndex < CommandActions.Num(); CommandIndex++)
		{
			EnhancedInputComponent->BindAction(CommandActions[CommandIndex], ETriggerEvent::Triggered, this, &AHktRtsPlayerController::HandleCommand, CommandIndex + 1);
		}
	}
}

void AHktRtsPlayerController::HandleLeftClick(const FInputActionValue& Value)
{
	// 유닛 선택 로직
	// 1. 마우스 커서 아래로 트레이스
	FHitResult HitResult;
	if (GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		AHktRtsUnit* ClickedUnit = Cast<AHktRtsUnit>(HitResult.GetActor());

		// TODO: 쉬프트 키 누름 여부에 따라 다중 선택/선택 해제 처리
		
		// 일단 단일 선택만 구현
		SelectedUnits.Empty();
		if (ClickedUnit)
		{
			SelectedUnits.Add(ClickedUnit);
			// TODO: 선택된 유닛 시각적 피드백 (예: 데칼 표시)
		}
	}
}

void AHktRtsPlayerController::HandleRightClick(const FInputActionValue& Value)
{
	// 유닛 명령 로직
	if (SelectedUnits.Num() > 0)
	{
		// 1. 마우스 커서 아래의 월드 위치 찾기 (주로 지형)
		FHitResult HitResult;
		if (GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
		{
			FVector TargetLocation = HitResult.Location;

			// 2. 서버에 명령 RPC 호출
			UnitCommandComponent->RequestMoveUnits(SelectedUnits, TargetLocation);
			
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, TargetLocation, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
		}
	}
}

void AHktRtsPlayerController::HandleZoom(const FInputActionValue& Value)
{
	if (AHktRtsCameraPawn* CameraPawn = Cast<AHktRtsCameraPawn>(GetPawn()))
	{
		CameraPawn->HandleZoom(Value);
	}
}

void AHktRtsPlayerController::HandleCommand(const FInputActionValue& Value, int32 CommandIndex)
{
	// 유닛 명령 로직
	if (SelectedUnits.Num() > 0)
	{
		// 1. 마우스 커서 아래의 월드 위치 찾기 (주로 지형)
	}
}
