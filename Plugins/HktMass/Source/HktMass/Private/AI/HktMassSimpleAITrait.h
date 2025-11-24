// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassEntityTraitBase.h"
#include "HktMassSimpleAITrait.generated.h"

//----------------------------------------------------------------------//
//  Traits
//----------------------------------------------------------------------//

/**
 * 지정된 태그를 가진 액터들의 위치를 찾아 순찰 경로로 설정하는 Trait
 */
UCLASS(MinimalAPI, meta = (DisplayName = "HKT Simple Patrol AI"))
class UHktMassSimpleAITrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	// 월드에 배치된 액터 중 이 태그를 가진 액터들을 찾아 순찰 지점으로 사용합니다.
	UPROPERTY(EditAnywhere, Category = "AI")
	FName PatrolActorTag;

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};