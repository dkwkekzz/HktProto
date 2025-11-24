// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Engine/EngineTypes.h"
#include "HktRtsPlayerController.generated.h"

class AHktRtsUnit;
class UInputMappingContext;
class UInputAction;
class UNiagaraSystem;
class UHktUnitCommandComponent;
class UHktConstructionComponent;
struct FInputActionValue;

/**
 * HktProto RTS 플레이어 컨트롤러
 * 유닛 선택, 명령 하달, 카메라 제어 등 RTS 관련 모든 입력을 처리합니다.
 */
UCLASS()
class HKTPROTO_API AHktRtsPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AHktRtsPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/** Enhanced Input 매핑 컨텍스트 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** 좌클릭 입력 액션 (유닛 선택) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LeftClickAction;

	/** 우클릭 입력 액션 (유닛 명령) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> RightClickAction;

	/** 휠 줌 입력 액션 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ZoomAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TArray<TObjectPtr<UInputAction>> CommandActions;

	/** FX Class that we will spawn when clicking */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UNiagaraSystem> FXCursor;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHktUnitCommandComponent> UnitCommandComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHktConstructionComponent> ConstructionComponent;

	/** 좌클릭 입력 처리 핸들러 */
	void HandleLeftClick(const FInputActionValue& Value);

	/** 우클릭 입력 처리 핸들러 */
	void HandleRightClick(const FInputActionValue& Value);

	/** 줌 입력 처리 핸들러 */
	void HandleZoom(const FInputActionValue& Value);
	
	/** 유닛 명령 입력 처리 핸들러 */
	void HandleCommand(const FInputActionValue& Value, int32 CommandIndex);

	/** BP에서 좌클릭 입력을 처리할 수 있도록 제공 */
	UFUNCTION(BlueprintImplementableEvent, Category = "RTS|Input")
	void BP_OnLeftClick(const FHitResult& HitResult);

	/** BP에서 우클릭 입력을 처리할 수 있도록 제공 */
	UFUNCTION(BlueprintImplementableEvent, Category = "RTS|Input")
	void BP_OnRightClick(const FHitResult& HitResult);

	/** BP에서 커맨드 입력을 처리할 수 있도록 제공 */
	UFUNCTION(BlueprintImplementableEvent, Category = "RTS|Input")
	void BP_OnCommandTriggered(int32 CommandIndex, const FHitResult& HitResult);

private:
	/** 현재 선택된 유닛 배열 (클라이언트 측에서만 관리) */
	TArray<TWeakObjectPtr<AHktRtsUnit>> SelectedUnits;
};
