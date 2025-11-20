// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HktMassSquadLeader.generated.h"

/**
 * 서버에서 리플리케이션되는 분대장 액터.
 * AIController에 의해 움직이며, 클라이언트의 MassEntity들은 이 액터를 따라다닙니다.
 */
UCLASS()
class HKTMASS_API AHktMassSquadLeader : public AActor
{
	GENERATED_BODY()

public:
	AHktMassSquadLeader();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// 이 분대장의 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad", Replicated)
	int32 SquadID = 1;
};