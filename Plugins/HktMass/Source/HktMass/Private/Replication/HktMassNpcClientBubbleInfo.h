// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassClientBubbleInfoBase.h"
#include "MassClientBubbleHandler.h"
#include "MassClientBubbleSerializerBase.h"
#include "HktMassNpcReplicationTypes.h"
#include "HktMassNpcClientBubbleInfo.generated.h"

//----------------------------------------------------------------------//
// FHktMassNpcClientBubbleHandler
//----------------------------------------------------------------------//

/**
 * NPC 복제 ?�들??(?�수 C++ ?�래?? USTRUCT ?�님)
 * TClientBubbleHandlerBase ?�플릿을 ?�속?�여 FastArray 관�?
 * ?�제 로직?� Helper ?�래?�에 ?�임?�여 ?�버/?�라?�언??코드 분리
 */
class HKTMASS_API FHktMassNpcClientBubbleHandler : public TClientBubbleHandlerBase<FHktReplicatedNpcAgentArrayItem>
{
public:
	typedef TClientBubbleHandlerBase<FHktReplicatedNpcAgentArrayItem> Super;

	// TClientBubbleHandlerBase??가???�수 구현
	
#if UE_REPLICATION_COMPILE_CLIENT_CODE
	/** ?�라?�언?? Agent가 ?�거?�기 ???�출 */
	virtual void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize) override;
	
	/** ?�라?�언?? Agent가 추�??????�출 */
	virtual void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize) override;
	
	/** ?�라?�언?? Agent가 변경된 ???�출 */
	virtual void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize) override;
#endif // UE_REPLICATION_COMPILE_CLIENT_CODE

#if UE_REPLICATION_COMPILE_SERVER_CODE
	/** ?�버: ??Agent 추�? */
	FMassReplicatedAgentHandle AddAgent(FMassEntityHandle Entity, FHktReplicatedNpcAgent& Agent);
	
	/** ?�버: Agent ?�정 */
	bool ModifyAgent(FMassReplicatedAgentHandle Handle, const FHktReplicatedNpcAgent& Agent, 
		EMassLOD::Type LOD, double Time, double LastUpdateTime, const float* UpdateIntervals);
	
	/** ?�버: Agent ?�거 */
	void RemoveAgent(FMassReplicatedAgentHandle Handle);
#endif // UE_REPLICATION_COMPILE_SERVER_CODE
};

//----------------------------------------------------------------------//
// FHktMassNpcClientBubbleSerializer
//----------------------------------------------------------------------//

/**
 * NPC FastArray 직렬??구조�?(USTRUCT)
 * FastArray�??�유?�고 NetDeltaSerialize�?구현?�여 ?�율?�인 ?��? ?�송 지??
 */
USTRUCT()
struct HKTMASS_API FHktMassNpcClientBubbleSerializer : public FMassClientBubbleSerializerBase
{
	GENERATED_BODY()

	FHktMassNpcClientBubbleSerializer();

	/** NetDeltaSerialize 구현 - FastArray???��? 직렬??*/
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams);

	/** Handler ?�근??*/
	FHktMassNpcClientBubbleHandler& GetBubbleHandler() { return Bubble; }

public:
	/** Handler (로직 처리) */
	FHktMassNpcClientBubbleHandler Bubble;

protected:
	/** FastArray ?�이??(?�제 복제?�는 Agent 배열) */
	UPROPERTY(Transient)
	TArray<FHktReplicatedNpcAgentArrayItem> Agents;

	friend FHktMassNpcClientBubbleHandler;
};

// NetDeltaSerializer 지???�성 ?�언
template<>
struct TStructOpsTypeTraits<FHktMassNpcClientBubbleSerializer> : public TStructOpsTypeTraitsBase2<FHktMassNpcClientBubbleSerializer>
{
	enum
	{
		WithNetDeltaSerializer = true,
		WithCopy = false,
	};
};

//----------------------------------------------------------------------//
// AHktMassNpcClientBubbleInfo
//----------------------------------------------------------------------//

/**
 * ?�라?�언?�별 NPC 복제 ?�터
 * �??�라?�언?�마???�버?�서 ?�나???�성??
 */
UCLASS()
class HKTMASS_API AHktMassNpcClientBubbleInfo : public AMassClientBubbleInfoBase
{
	GENERATED_BODY()

public:
	AHktMassNpcClientBubbleInfo(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Serializer ?�근??*/
	FHktMassNpcClientBubbleSerializer& GetNpcSerializer() { return NpcSerializer; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** NPC Serializer (복제?�는 FastArray ?�유) */
	UPROPERTY(Replicated, Transient)
	FHktMassNpcClientBubbleSerializer NpcSerializer;
};

