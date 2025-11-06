// HktConstructionComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HktConstructionComponent.generated.h"

/**
 * 유닛 및 건물 생성을 담당하는 컴포넌트입니다.
 * PlayerController에 부착되며, 서버에서 실제 생성을 처리합니다.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HKTPROTO_API UHktConstructionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHktConstructionComponent();

protected:
	virtual void InitializeComponent() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** [Client] UIGlobalsSubsystem의 델리게이트를 구독하여 이 함수를 호출합니다. */
	UFUNCTION()
	void HandleSpawnRequest(TSubclassOf<APawn> UnitClass);

public:
	/** * [Client] 클라이언트가 서버에 유닛 생성을 요청합니다.
	 * (HandleSpawnRequest 내부에서 호출되거나, UI 없이 직접 호출될 수 있습니다)
	 * @param UnitClass - 생성할 유닛의 클래스
	 * @param SpawnLocation - 생성할 위치 (RTS에서는 보통 집결지 또는 건물 위치)
	 */
	void RequestSpawnUnit(TSubclassOf<APawn> UnitClass, const FVector& SpawnLocation);

protected:
	/** * [Server] 유닛 생성 요청을 서버에서 실행합니다.
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestSpawnUnit(TSubclassOf<APawn> UnitClass, const FVector& SpawnLocation);
	bool Server_RequestSpawnUnit_Validate(TSubclassOf<APawn> UnitClass, const FVector& SpawnLocation);
	void Server_RequestSpawnUnit_Implementation(TSubclassOf<APawn> UnitClass, const FVector& SpawnLocation);

	/** * [Client] 서버가 생성 요청 실패 시 클라이언트에 알립니다.
	 */
	UFUNCTION(Client, Reliable)
	void Client_NotifySpawnFailed(const FString& Reason);
	void Client_NotifySpawnFailed_Implementation(const FString& Reason);

	/** [Server] 실제 유닛을 스폰하는 서버 전용 함수 */
	void SpawnUnitInternal(TSubclassOf<APawn> UnitClass, const FVector& SpawnLocation);
};

