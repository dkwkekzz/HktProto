// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassSquadAgentTrait.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassReplicationTrait.h"

void UHktMassSquadAgentTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FTransformFragment>();

	FHktMassSquadFragment& SquadFrag = BuildContext.AddFragment_GetRef<FHktMassSquadFragment>();
	SquadFrag.MemberCount = MemberCount;
	SquadFrag.MemberConfig = MemberConfig;

	BuildContext.AddTag<FHktMassSquadDebugVisualizationTag>();

	// 복제 설정 추가 (서버에서만 Replicator 동작)
	// MassReplicationTrait 등을 통해 추가하거나 직접 Config에 추가할 수 있음.
	// 여기서는 일반 Trait처럼 Replicator를 추가하는 방식이 아니라, 
	// MassReplicationTrait를 사용하여 Replicator Class를 지정하는 것이 일반적임.
	// 하지만 Custom Trait 내부에서 처리하려면 AddFragment가 아니라 별도 설정이 필요함.
	
	// 보통은 EntityConfigAsset에서 MassReplicationTrait를 추가하고 거기서 Replicator를 설정함.
	// 하지만 편의상 이 Trait가 Replicator 의존성을 가질 수 있다면 좋음.
}
