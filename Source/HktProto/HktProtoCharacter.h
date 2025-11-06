// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "HktProtoCharacter.generated.h"

class UHktStateTreeComponent;

UCLASS(Blueprintable)
class AHktProtoCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AHktProtoCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

private:
	/** On State Tag Changed */
	UFUNCTION()
	void OnStateTagChanged(FGameplayTag Tag, bool bIsAdded);

	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** State Tree Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HktProto", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHktStateTreeComponent> StateTreeComponent;
};

