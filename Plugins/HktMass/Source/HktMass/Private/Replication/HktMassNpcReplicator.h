// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassReplicationProcessor.h"
#include "HktMassNpcReplicator.generated.h"

/**
 * HktProto NPC ë³µì œ ë¡œì§ ì²˜ë¦¬ ?´ë˜??
 * UMassReplicatorBaseë¥??ì†?˜ì—¬ ?œë²„?ì„œ NPC ?°ì´?°ë? ?˜ì§‘?˜ê³  ?´ë¼?´ì–¸?¸ì— ë³µì œ
 */
UCLASS()
class HKTMASS_API UHktMassNpcReplicator : public UMassReplicatorBase
{
	GENERATED_BODY()

public:
	UHktMassNpcReplicator();

	/** Fragment ?”êµ¬?¬í•­ ì¶”ê? */
	virtual void AddRequirements(FMassEntityQuery& EntityQuery) override;

	/** ?´ë¼?´ì–¸??ë³µì œ ì²˜ë¦¬ */
	virtual void ProcessClientReplication(FMassExecutionContext& Context, FMassReplicationContext& ReplicationContext) override;
};

