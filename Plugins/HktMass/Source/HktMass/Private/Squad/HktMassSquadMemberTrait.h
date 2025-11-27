// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassEntityTraitBase.h"
#include "HktMassSquadFragments.h"
#include "HktMassSquadMemberTrait.generated.h"

/**
 * 분대원(Squad Member)을 위한 Trait.
 * 분대 소속 정보와 이동 관련 컴포넌트를 추가합니다.
 */
UCLASS(meta=(DisplayName="Hkt Squad Member"))
class UHktMassSquadMemberTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};

