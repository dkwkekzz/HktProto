// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassReplicationTypes.h"
#include "MassClientBubbleHandler.h"
#include "MassEntityConfigAsset.h"
#include "HktMassSquadReplicationTypes.generated.h"

/**
 * 네트워크로 전송되는 Squad 에이전트 데이터 구조체
 * FReplicatedAgentBase를 상속하여 NetID와 TemplateID를 자동으로 포함
 */
USTRUCT()
struct HKTMASS_API FHktReplicatedSquadAgent : public FReplicatedAgentBase
{
	GENERATED_BODY()

	// 분대 위치
	UPROPERTY()
	FVector Position = FVector::ZeroVector;

	// 분대 상태 (이동, 전투, 대기 등)
	UPROPERTY()
	uint8 SquadState = 0;

	// 멤버 구성 설정 (EntityConfig 에셋)
	// 참고: UObject 포인터는 직접 복제되지 않을 수 있으므로, 
	// 실제로는 SoftPath나 ID로 관리하거나, 서버/클라가 동일한 에셋을 로드하고 있어야 함.
	// 여기서는 일단 직접 참조하되, 필요시 패스 문자열이나 인덱스로 변경 고려.
	UPROPERTY()
	TObjectPtr<UMassEntityConfigAsset> MemberConfig = nullptr;

	// 멤버 수
	UPROPERTY()
	int32 MemberCount = 0;
};

/**
 * FastArray 항목 - FMassFastArrayItemBase 상속
 * Agent 멤버 변수는 반드시 "Agent"라는 이름으로 선언해야 함 (MassReplication 규약)
 */
USTRUCT()
struct HKTMASS_API FHktReplicatedSquadAgentArrayItem : public FMassFastArrayItemBase
{
	GENERATED_BODY()

	FHktReplicatedSquadAgentArrayItem() = default;

	FHktReplicatedSquadAgentArrayItem(const FHktReplicatedSquadAgent& InAgent, FMassReplicatedAgentHandle InHandle)
		: FMassFastArrayItemBase(InHandle)
		, Agent(InAgent)
	{
	}

	typedef FHktReplicatedSquadAgent FReplicatedAgentType;

	UPROPERTY()
	FHktReplicatedSquadAgent Agent;
};

