// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HktEntityManager.h"
#include "HktFlowVM.h"
#include "HktServiceInterface.h"
#include "HktSimulationSubsystem.generated.h"

struct FHktIntentEvent;

// 로깅 카테고리 선언
DECLARE_LOG_CATEGORY_EXTERN(LogHktSimulation, Log, All);

/**
 * World Subsystem for managing HktSimulation
 * - IntentEvent를 받아 FlowVM을 통해 시뮬레이션을 실행
 * - Entity/Player Database를 관리
 * - IHktSimulationProvider 구현으로 외부 모듈에 서비스 제공
 * 
 * 의존성 방향: HktSimulation → HktService (인터페이스) ← HktIntent
 */
class FHktVMPool;

UCLASS()
class HKTSIMULATION_API UHktSimulationSubsystem : public UTickableWorldSubsystem, public IHktSimulationProvider
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

	// --- IHktSimulationProvider 구현 ---
	
	/** 새 플레이어 등록 */
	virtual FHktPlayerHandle RegisterPlayer() override;
	
	/** 플레이어 등록 해제 */
	virtual void UnregisterPlayer(const FHktPlayerHandle& Handle) override;
	
	/** 플레이어 속성 스냅샷 조회 (Late Join용) */
	virtual bool GetPlayerSnapshot(const FHktPlayerHandle& Handle, TArray<float>& OutValues) const override;
	
	/** 플레이어 속성 스냅샷으로 초기화 */
	virtual void InitializePlayerFromSnapshot(const FHktPlayerHandle& Handle, const TArray<float>& Values) override;

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

	// --- Lockstep Event Management ---
	
	/** 현재 처리 중인 마지막 EventId 조회 */
	int32 GetLastProcessedEventId() const { return LastProcessedEventId; }

	// --- Player Attribute API (Lockstep 방식 - Sink 제거) ---
	
	/** 플레이어 속성 설정 (로컬 DB만 업데이트, 서버는 별도로 FAS 동기화) */
	void SetPlayerAttribute(const FHktPlayerHandle& Handle, EHktAttributeType Type, float Value);
	
	/** 플레이어 속성 수정 (Delta 적용, 로컬 DB만 업데이트) */
	void ModifyPlayerAttribute(const FHktPlayerHandle& Handle, EHktAttributeType Type, float Delta);

	// --- Lockstep Synchronization ---
	
	/** Simulation이 실행 중인지 체크 (Late Join 감지용) */
	bool IsSimulationRunning() const { return bSimulationRunning; }

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

	// [Optimized] VM Pool for reusing VM instances
	TUniquePtr<FHktVMPool> VMPool;

	// Active VMs (now just pointers, owned by pool)
	TArray<FHktFlowVM*> ActiveVMs;

	// [Optimized] External -> Internal Handle Mapping only
	// Reverse mapping (Internal -> External) is stored in EntityDatabase.ExternalIds
	TMap<int32, FUnitHandle> ExternalToInternalMap;

	// 다음 외부 ID 생성용 카운터
	int32 NextExternalUnitId = 1;

	// --- Lockstep Event Processing ---
	
	// 마지막 처리 EventId 추적
	int32 LastProcessedEventId = 0;
	
	// 시뮬레이션 중 속성 변경 누적
	FHktSimulationResult PendingResult;

	// --- Lockstep Synchronization ---
	
	// Simulation 실행 상태 플래그 (Late Join 감지용)
	bool bSimulationRunning = false;
	
	// IHktAttributeSink 캐시 (더 이상 사용하지 않음 - Late Join용 FAS만 서버에서 관리)
	mutable IHktAttributeSink* CachedAttributeSink = nullptr;
	
	/** Sink 조회 헬퍼 (더 이상 푸시용으로 사용 안 함) */
	IHktAttributeSink* GetAttributeSink() const;
};
