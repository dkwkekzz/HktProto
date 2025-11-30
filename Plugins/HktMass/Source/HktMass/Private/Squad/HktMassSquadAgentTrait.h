// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "HktMassSquadFragments.h"
#include "HktMassSquadAgentTrait.generated.h"

/**
 * 분대(Squad) 자체를 나타내는 Agent를 위한 Trait.
 * 분대원 정보를 관리하고 분대의 중심 위치를 가집니다.
 */
UCLASS(meta=(DisplayName="Hkt Squad Agent"))
class UHktMassSquadAgentTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	UPROPERTY(EditAnywhere, Category = "Squad")
	int32 MemberCount = 3;

	UPROPERTY(EditAnywhere, Category = "Squad")
	float SquadMaxRadius = 500.f;

	UPROPERTY(EditAnywhere, Category = "Squad")
	TObjectPtr<class UMassEntityConfigAsset> MemberConfig;
};

