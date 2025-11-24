#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "HktMassSquadCommandComponent.generated.h"

class UHktMassSquadSubsystem;

/**
 * Mass 분대 명령을 담당하는 경량 ActorComponent.
 * 서버에서 리플리케이션되며 다양한 액터에 손쉽게 부착할 수 있습니다.
 */
UCLASS(Blueprintable, ClassGroup = (Mass), meta = (BlueprintSpawnableComponent))
class HKTMASS_API UHktMassSquadCommandComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHktMassSquadCommandComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 외부에서 분대 위치를 업데이트할 때 호출 */
	UFUNCTION(BlueprintCallable, Category = "Squad")
	void UpdateSquadLocation(const FVector& NewLocation);

protected:
	/** 이 분대의 고유 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad", Replicated)
	int32 SquadID = 1;

private:
	/** 클라이언트와 동기화되는 분대 위치 */
	UPROPERTY(Transient, Replicated)
	FVector SquadLocation = FVector::ZeroVector;
};

