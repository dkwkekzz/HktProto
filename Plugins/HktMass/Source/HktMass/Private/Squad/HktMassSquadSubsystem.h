// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HktMassSquadSubsystem.generated.h"

class UHktMassSquadCommandComponent;

UCLASS()
class HKTMASS_API UHktMassSquadSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	// 분대 커맨드 컴포넌트 등록/해제
	void RegisterSquadCommandComponent(int32 SquadID, UHktMassSquadCommandComponent* SquadComponent);
	void UnregisterSquadCommandComponent(int32 SquadID);

	// ID로 분대 커맨드 컴포넌트 가져오기
	UHktMassSquadCommandComponent* GetSquadCommandComponent(int32 SquadID) const;

private:
	UPROPERTY()
	TMap<int32, UHktMassSquadCommandComponent*> SquadComponentMap;
};
