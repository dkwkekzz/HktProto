// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassEntityTraitBase.h"
#include "HktMassSquadFragments.h"
#include "HktMassSquadTrait.generated.h"

//----------------------------------------------------------------------//
//  Traits
//----------------------------------------------------------------------//

/**
 * 지정된 태그를 가진 액터들의 위치를 찾아 순찰 경로로 설정하는 Trait
 */
UCLASS(MinimalAPI)
class UHktMassSquadTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	// 소속될 분대 ID (0이면 소속 없음)
	UPROPERTY(EditAnywhere)
	int32 SquadID = 0;

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};