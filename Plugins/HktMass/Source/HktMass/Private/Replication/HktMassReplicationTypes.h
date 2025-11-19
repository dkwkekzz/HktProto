// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassReplicationTypes.h"
#include "MassSpawnerTypes.h"
#include "MassClientBubbleHandler.h"
#include "HktMassReplicationTypes.generated.h"

/**
 * ?�트?�크�??�송?�는 NPC ?�이??구조�?
 * FReplicatedAgentBase�??�속?�여 NetID?� TemplateID�??�동?�로 ?�함
 */
USTRUCT()
struct HKTMASS_API FHktReplicatedNpcAgent : public FReplicatedAgentBase
{
	GENERATED_BODY()

	// Transform ?�이??
	UPROPERTY()
	FVector Position = FVector::ZeroVector;

	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;

	// AI ?�태 (0: Idle, 1: Patrol, 2: Chase, 3: Attack, 4: Dead)
	UPROPERTY()
	uint8 CurrentState = 0;

	// ?�재 체력
	UPROPERTY()
	float CurrentHealth = 100.0f;

	// ?�동 ?�도 (보간 ?�한 필수)
	UPROPERTY()
	FVector Velocity = FVector::ZeroVector;

	// ?�고: AnimationStateIndex??복제?��? ?�음
	// → ?�라?�언?�에???�도(Velocity)?� ?�태(State)�??�탕?�로 ?�체 계산
};

/**
 * FastArray ??�� - FMassFastArrayItemBase�??�속
 * Agent 멤버 변?�는 반드??"Agent"?�는 ?�름?�로 ?�언?�야 ??(MassReplication 규약)
 */
USTRUCT()
struct HKTMASS_API FHktReplicatedNpcAgentArrayItem : public FMassFastArrayItemBase
{
	GENERATED_BODY()

	// FastArray�??�한 기본 ?�성??
	FHktReplicatedNpcAgentArrayItem() = default;

	// ?�버?�서 ?�용?�는 ?�성??
	FHktReplicatedNpcAgentArrayItem(const FHktReplicatedNpcAgent& InAgent, FMassReplicatedAgentHandle InHandle)
		: FMassFastArrayItemBase(InHandle)
		, Agent(InAgent)
	{
	}

	// 복제??Agent ?�???�의 (TClientBubbleHandlerBase?�서 ?�용)
	typedef FHktReplicatedNpcAgent FReplicatedAgentType;

	// ?�제 복제?�는 Agent ?�이??- 반드??"Agent"?�는 ?�름?�어????
	UPROPERTY()
	FHktReplicatedNpcAgent Agent;
};
