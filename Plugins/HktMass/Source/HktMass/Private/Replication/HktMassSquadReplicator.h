// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassReplicationProcessor.h"
#include "HktMassSquadReplicator.generated.h"

/**
 * Squad Entity 복제 로직 처리 Replicator
 * UMassReplicatorBase를 상속하여 서버에서 Squad 데이터를 수집하고 클라이언트에 복제
 */
UCLASS()
class HKTMASS_API UHktMassSquadReplicator : public UMassReplicatorBase
{
	GENERATED_BODY()

public:
	UHktMassSquadReplicator();

protected:
	/** Fragment 요구사항 추가 */
	virtual void AddRequirements(FMassEntityQuery& EntityQuery) override;

	/** 클라이언트 복제 처리 (서버 -> 클라이언트 데이터 복사) */
	virtual void ProcessClientReplication(FMassExecutionContext& Context, FMassReplicationContext& ReplicationContext) override;
};

