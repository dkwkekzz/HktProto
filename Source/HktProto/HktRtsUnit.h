// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HktRtsUnit.generated.h"

class UHktStateTreeComponent;

/**
 * RTS에서 컨트롤 가능한 기본 유닛.
 * 리플리케이션을 통해 상태와 이동이 동기화됩니다.
 * 수천 개가 될 수 있으므로 최적화가 중요하지만, 프로토타입에서는 ACharacter를 사용합니다.
 */
UCLASS()
class HKTPROTO_API AHktRtsUnit : public ACharacter
{
	GENERATED_BODY()

public:
	AHktRtsUnit();

protected:
	virtual void BeginPlay() override;

	/** StateTree 로직을 관리하는 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StateTree")
	TObjectPtr<UHktStateTreeComponent> StateTreeComponent;

public:	
	virtual void Tick(float DeltaTime) override;

	/**
	 * [서버 전용] 이 유닛을 특정 위치로 이동시킵니다.
	 * PlayerController의 서버 RPC를 통해 호출됩니다.
	 * @param TargetLocation 이동할 목표 월드 위치
	 */
	void MoveUnitTo(const FVector& TargetLocation);

	// 리플리케이션 설정
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// TODO: StateTree가 변경할 유닛의 상태 (예: EUnitState)를 Replicated 속성으로 추가
	// UPROPERTY(ReplicatedUsing = OnRep_CurrentState)
	// EUnitState CurrentState;

	// UFUNCTION()
	// void OnRep_CurrentState();
};
