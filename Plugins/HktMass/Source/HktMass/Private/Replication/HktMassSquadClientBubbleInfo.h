// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassClientBubbleInfoBase.h"
#include "MassClientBubbleHandler.h"
#include "MassClientBubbleSerializerBase.h"
#include "HktMassSquadReplicationTypes.h"
#include "HktMassSquadClientBubbleInfo.generated.h"

//----------------------------------------------------------------------//
// FHktMassSquadClientBubbleHandler
//----------------------------------------------------------------------//

/**
 * Squad 복제 핸들러
 * TClientBubbleHandlerBase 템플릿 상속
 */
class HKTMASS_API FHktMassSquadClientBubbleHandler : public TClientBubbleHandlerBase<FHktReplicatedSquadAgentArrayItem>
{
public:
	typedef TClientBubbleHandlerBase<FHktReplicatedSquadAgentArrayItem> Super;

#if UE_REPLICATION_COMPILE_CLIENT_CODE
	virtual void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize) override;
	virtual void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize) override;
#endif // UE_REPLICATION_COMPILE_CLIENT_CODE

#if UE_REPLICATION_COMPILE_SERVER_CODE
	FMassReplicatedAgentHandle AddAgent(FMassEntityHandle Entity, FHktReplicatedSquadAgent& Agent);
	bool ModifyAgent(FMassReplicatedAgentHandle Handle, const FHktReplicatedSquadAgent& Agent, 
		EMassLOD::Type LOD, double Time, double LastUpdateTime, const float* UpdateIntervals);
	void RemoveAgent(FMassReplicatedAgentHandle Handle);
#endif // UE_REPLICATION_COMPILE_SERVER_CODE

private:
#if UE_REPLICATION_COMPILE_CLIENT_CODE
	void UpdateEntityFragments(const FMassEntityView& EntityView, const FHktReplicatedSquadAgent& Agent);
#endif // UE_REPLICATION_COMPILE_CLIENT_CODE
};

//----------------------------------------------------------------------//
// FHktMassSquadClientBubbleSerializer
//----------------------------------------------------------------------//

/**
 * Squad FastArray 직렬화 구조체
 */
USTRUCT()
struct HKTMASS_API FHktMassSquadClientBubbleSerializer : public FMassClientBubbleSerializerBase
{
	GENERATED_BODY()

	FHktMassSquadClientBubbleSerializer();

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams);

	FHktMassSquadClientBubbleHandler& GetBubbleHandler() { return Bubble; }

public:
	FHktMassSquadClientBubbleHandler Bubble;

protected:
	UPROPERTY(Transient)
	TArray<FHktReplicatedSquadAgentArrayItem> Agents;

	friend FHktMassSquadClientBubbleHandler;
};

template<>
struct TStructOpsTypeTraits<FHktMassSquadClientBubbleSerializer> : public TStructOpsTypeTraitsBase2<FHktMassSquadClientBubbleSerializer>
{
	enum
	{
		WithNetDeltaSerializer = true,
		WithCopy = false,
	};
};

//----------------------------------------------------------------------//
// AHktMassSquadClientBubbleInfo
//----------------------------------------------------------------------//

/**
 * Squad 전용 Bubble Info Actor
 */
UCLASS(Blueprintable)
class HKTMASS_API AHktMassSquadClientBubbleInfo : public AMassClientBubbleInfoBase
{
	GENERATED_BODY()

public:
	AHktMassSquadClientBubbleInfo(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	FHktMassSquadClientBubbleSerializer& GetSquadSerializer() { return SquadSerializer; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(Replicated, Transient)
	FHktMassSquadClientBubbleSerializer SquadSerializer;
};

