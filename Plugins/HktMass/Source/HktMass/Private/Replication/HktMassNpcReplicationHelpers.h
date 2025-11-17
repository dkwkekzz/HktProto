// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassCommonTypes.h"
#include "MassReplicationTypes.h"
#include "HktMassNpcReplicationTypes.h"

struct FMassEntityView;
struct FMassClientBubbleSerializerBase;
struct FMassExecutionContext;

/**
 * ?�버 �?복제 ?�퍼 ?�래??
 * FastArray 조작 로직??캡슐?�하??매크�?분기 ?�거
 */
class HKTMASS_API FHktMassNpcServerReplicationHelper
{
public:
	/**
	 * FastArray????Agent 추�?
	 * @param Agents FastArray 참조
	 * @param Serializer FastArray 직렬??객체
	 * @param Entity Mass ?�티???�들
	 * @param Agent 복제??Agent ?�이??
	 * @return ?�성??Agent???�들
	 */
	static FMassReplicatedAgentHandle AddAgent(
		TArray<FHktReplicatedNpcAgentArrayItem>& Agents,
		FMassClientBubbleSerializerBase& Serializer,
		FMassEntityHandle Entity,
		const FHktReplicatedNpcAgent& Agent);

	/**
	 * FastArray??Agent ?�정
	 * LOD???�라 ?�데?�트 간격??체크?�여 ?�요??경우?�만 ?�정
	 * @param Agents FastArray 참조
	 * @param Serializer FastArray 직렬??객체
	 * @param Handle ?�정??Agent???�들
	 * @param Agent ?�로??Agent ?�이??
	 * @param LOD ?�재 LOD ?�벨
	 * @param Time ?�재 ?�간
	 * @param LastUpdateTime 마�?�??�데?�트 ?�간
	 * @param UpdateIntervals LOD�??�데?�트 간격 배열
	 * @return ?�데?�트 ?�으�?true
	 */
	static bool ModifyAgent(
		TArray<FHktReplicatedNpcAgentArrayItem>& Agents,
		FMassClientBubbleSerializerBase& Serializer,
		FMassReplicatedAgentHandle Handle,
		const FHktReplicatedNpcAgent& Agent,
		EMassLOD::Type LOD,
		double Time,
		double LastUpdateTime,
		const float* UpdateIntervals);

	/**
	 * FastArray?�서 Agent ?�거
	 * @param Agents FastArray 참조
	 * @param Serializer FastArray 직렬??객체
	 * @param Handle ?�거??Agent???�들
	 */
	static void RemoveAgent(
		TArray<FHktReplicatedNpcAgentArrayItem>& Agents,
		FMassClientBubbleSerializerBase& Serializer,
		FMassReplicatedAgentHandle Handle);
};

/**
 * ?�라?�언??�?복제 ?�퍼 ?�래??
 * ?�티???�성/?�정/?�거 로직??캡슐??
 */
class HKTMASS_API FHktMassNpcClientReplicationHelper
{
public:
	/**
	 * FastArray??Agent가 추�??�었?????�출
	 * ???�티?��? ?�폰?�거??기존 ?�티?��? ?�데?�트
	 * @param AddedIndices 추�?????��???�덱??배열
	 * @param Agents FastArray 참조
	 * @param Serializer FastArray 직렬??객체
	 */
	static void HandleItemAdded(
		const TArrayView<int32> AddedIndices,
		TArray<FHktReplicatedNpcAgentArrayItem>& Agents,
		FMassClientBubbleSerializerBase& Serializer);

	/**
	 * FastArray??Agent가 변경되?�을 ???�출
	 * 기존 ?�티?�의 Fragment�??�데?�트
	 * @param ChangedIndices 변경된 ??��???�덱??배열
	 * @param Agents FastArray 참조
	 * @param Serializer FastArray 직렬??객체
	 */
	static void HandleItemChanged(
		const TArrayView<int32> ChangedIndices,
		TArray<FHktReplicatedNpcAgentArrayItem>& Agents,
		FMassClientBubbleSerializerBase& Serializer);

	/**
	 * FastArray?�서 Agent가 ?�거?�었?????�출
	 * ?�티?��? ?�거?�고 ?�리
	 * @param RemovedIndices ?�거????��???�덱??배열
	 * @param Agents FastArray 참조
	 * @param Serializer FastArray 직렬??객체
	 */
	static void HandleItemRemoved(
		const TArrayView<int32> RemovedIndices,
		TArray<FHktReplicatedNpcAgentArrayItem>& Agents,
		FMassClientBubbleSerializerBase& Serializer);

private:
	/**
	 * ?�티?�의 Fragment�?Agent ?�이?�로 ?�데?�트 (공통 로직)
	 * @param EntityView ?�데?�트???�티??�?
	 * @param Agent Agent ?�이??
	 */
	static void UpdateEntityFragments(const FMassEntityView& EntityView, const FHktReplicatedNpcAgent& Agent);
};

