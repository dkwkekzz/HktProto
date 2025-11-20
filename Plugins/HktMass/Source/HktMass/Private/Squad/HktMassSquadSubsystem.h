// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HktMassSquadSubsystem.generated.h"

class AHktMassSquadLeader;

UCLASS()
class HKTMASS_API UHktMassSquadSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	// 분대장 등록/해제
	void RegisterSquadLeader(int32 SquadID, AHktMassSquadLeader* SquadLeader);
	void UnregisterSquadLeader(int32 SquadID);

	// ID로 분대장 위치 가져오기
	AHktMassSquadLeader* GetSquadLeader(int32 SquadID) const;

private:
	UPROPERTY()
	TMap<int32, AHktMassSquadLeader*> SquadLeaderMap;
};
