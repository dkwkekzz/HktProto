// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HktEntityManager.h"
#include "HktFlowVM.h"
#include "HktSimulationSubsystem.generated.h"

struct FHktIntentEvent;
struct FHktIntentEventEntry;

// 로깅 카테고리 선언
DECLARE_LOG_CATEGORY_EXTERN(LogHktSimulation, Log, All);

/**
 * World Subsystem for managing HktSimulation
 * - IntentEvent를 받아 FlowVM을 통해 시뮬레이션을 실행
 * - Entity/Player Database를 관리
 * 
 * Sliding Window 방식:
 * - FetchNewEvents로 커서 이후의 이벤트만 조회
 * - 처리 성공 시 커서 업데이트
 * - 실패 시 커서 유지하여 다음 프레임에 재시도
 */
UCLASS()
class HKTSIMULATION_API UHktSimulationSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// UTickableWorldSubsystem Interface
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	// Helper to get the subsystem from a world context
	static UHktSimulationSubsystem* Get(const UObject* WorldContextObject);

	// --- Entity Management API ---
	
	/** 외부 UnitHandle을 내부 핸들로 변환 (필요시 생성) */
	FUnitHandle GetOrCreateInternalHandle(int32 ExternalUnitId, FVector Location = FVector::ZeroVector, FRotator Rotation = FRotator::ZeroRotator);
	
	/** 내부 핸들을 외부 ID로 변환 */
	int32 GetExternalUnitId(FUnitHandle InternalHandle) const;

	// --- Direct Access (for debugging/testing) ---
	
	FHktEntityManager& GetEntityManager() { return EntityManager; }
	const FHktEntityManager& GetEntityManager() const { return EntityManager; }

	// --- VM Management ---
	
	/** IntentEvent로부터 FlowVM을 생성하여 실행 큐에 추가 */
	void ExecuteIntentEvent(const FHktIntentEvent& Event);

	// --- Cursor Management (Sliding Window) ---
	
	/** 특정 채널의 현재 커서 조회 */
	int64 GetChannelCursor(int32 ChannelId) const;
	
	/** 특정 채널의 커서 강제 설정 (Late Join 시 사용) */
	void SetChannelCursor(int32 ChannelId, int64 NewCursor);

protected:
	/** 매 틱마다 IntentEvent를 수집하고 처리 (Sliding Window 방식) */
	void ProcessIntentEvents(float DeltaTime);

	/** 활성 VM들을 틱 */
	void TickActiveVMs(float DeltaTime);

	/** 완료된 VM들을 정리 */
	void CleanupFinishedVMs();

	/** IntentEvent의 EventTag에 따라 바이트코드를 빌드 */
	void BuildBytecodeForEvent(FHktFlowVM& VM, const FHktIntentEvent& Event);

private:
	// Global Entity Manager
	FHktEntityManager EntityManager;

	// Active VMs
	TArray<TUniquePtr<FHktFlowVM>> ActiveVMs;

	// External <-> Internal Handle Mapping
	TMap<int32, FUnitHandle> ExternalToInternalMap;
	TMap<int32, int32> InternalToExternalMap; // Index -> ExternalId

	// 다음 외부 ID 생성용 카운터
	int32 NextExternalUnitId = 1;

	// --- Sliding Window Cursors ---
	// Key: ChannelId, Value: LastProcessedSequenceId
	TMap<int32, int64> ChannelCursors;
};
