// HktUnitCommandComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HktUnitCommandComponent.generated.h"

class APawn;
class AHktRtsUnit;

/**
 * 선택된 유닛들에게 명령(이동, 공격 등)을 내리는 컴포넌트입니다.
 * PlayerController에 부착됩니다.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HKTPROTO_API UHktUnitCommandComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHktUnitCommandComponent();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	/** * [Client] 클라이언트가 서버에 유닛 이동을 요청합니다.
	 * @param SelectedUnits - 이동할 유닛들의 배열
	 * @param TargetLocation - 이동 목표 위치
	 */
	void RequestMoveUnits(const TArray<TWeakObjectPtr<AHktRtsUnit>>& InSelectedUnits, const FVector& TargetLocation);

protected:
	/** * [Server] 유닛 이동 요청을 서버에서 실행합니다.
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestMoveUnits(const TArray<APawn*>& InSelectedUnits, const FVector& TargetLocation);
	bool Server_RequestMoveUnits_Validate(const TArray<APawn*>& InSelectedUnits, const FVector& TargetLocation);
	void Server_RequestMoveUnits_Implementation(const TArray<APawn*>& InSelectedUnits, const FVector& TargetLocation);

	/** [Server] 유닛 소유권 및 이동 가능 여부 검사 */
	bool ValidateUnitCommand(APawn* Unit, const FVector& Location);
};
