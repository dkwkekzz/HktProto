// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HktEntityManager.h"
#include "HktFlowVM.h"
#include "HktServiceInterface.h"
#include "HktSimulationSubsystem.generated.h"

struct FHktIntentEvent;
struct FHktIntentEventBatch;
class UHktSimulationStashComponent;

// 로깅 카테고리 선언
DECLARE_LOG_CATEGORY_EXTERN(LogHktSimulation, Log, All);

/**
 * World Subsystem for managing HktSimulation
 * 
 * 새로운 설계:
 * - 모든 UHktSimulationStashComponent를 순회하며 이벤트 처리
 * - IntentEventProvider 의존성 제거
 * - 캐시 적중률 최적화를 위한 배치 처리 구조
 * 
 * 데이터 흐름:
 * StashComponent → GetPendingBatches() → ProcessBatch() → StoreResult()
 */
class FHktVMPool;

UCLASS()
class HKTSIMULATION_API UHktSimulationProcessSubsystem : public UTickableWorldSubsystem, public IHktSimulationProvider
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
	static UHktSimulationProcessSubsystem* Get(const UObject* WorldContextObject);

	// --- IHktSimulationProvider 구현 ---
	
	/** 새 플레이어 등록 */
	virtual FHktPlayerHandle RegisterPlayer() override;
	
	/** 플레이어 등록 해제 */
	virtual void UnregisterPlayer(const FHktPlayerHandle& Handle) override;
	
	/** 플레이어 속성 스냅샷 조회 (Late Join용) */
	virtual bool GetPlayerSnapshot(const FHktPlayerHandle& Handle, TArray<float>& OutValues) const override;
	
	/** 플레이어 속성 스냅샷으로 초기화 */
	virtual void InitializePlayerFromSnapshot(const FHktPlayerHandle& Handle, const TArray<float>& Values) override;

	// --- StashComponent Management ---
	
	/** StashComponent 등록 (BeginPlay에서 자동 호출) */
	void RegisterStashComponent(UHktSimulationStashComponent* StashComponent);
	
	/** StashComponent 등록 해제 (EndPlay에서 자동 호출) */
	void UnregisterStashComponent(UHktSimulationStashComponent* StashComponent);
	
	/** 등록된 StashComponent 목록 조회 */
	const TArray<TWeakObjectPtr<UHktSimulationStashComponent>>& GetRegisteredStashComponents() const 
	{ 
		return RegisteredStashComponents; 
	}

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
	
	/** 현재 처리 중인 마지막 프레임 번호 조회 */
	int32 GetLastProcessedFrameNumber() const { return LastProcessedFrameNumber; }

	// --- Player Attribute API ---
	
	/** 플레이어 속성 설정 (로컬 DB만 업데이트) */
	void SetPlayerAttribute(const FHktPlayerHandle& Handle, EHktAttributeType Type, float Value);
	
	/** 플레이어 속성 수정 (Delta 적용) */
	void ModifyPlayerAttribute(const FHktPlayerHandle& Handle, EHktAttributeType Type, float Delta);

	// --- Synchronization ---
	
	/** Simulation이 실행 중인지 체크 (Late Join 감지용) */
	bool IsSimulationRunning() const { return bSimulationRunning; }

protected:
	/** 모든 StashComponent를 순회하며 이벤트 처리 */
	void ProcessStashComponents(float DeltaTime);
	
	/** 단일 StashComponent 처리 */
	void ProcessStashComponent(UHktSimulationStashComponent* StashComponent);
	
	/** 배치 내의 이벤트들을 처리 */
	void ProcessBatch(const FHktIntentEventBatch& Batch, FHktSimulationResult& OutResult);

	/** 활성 VM들을 틱 */
	void TickActiveVMs(float DeltaTime);

	/** 완료된 VM들을 정리 */
	void CleanupFinishedVMs();

	/** IntentEvent의 EventTag에 따라 바이트코드를 빌드 */
	void BuildBytecodeForEvent(FHktFlowVM& VM, const FHktIntentEvent& Event);

private:
	// --- StashComponent Management ---
	
	/** 등록된 StashComponent 목록 */
	UPROPERTY()
	TArray<TWeakObjectPtr<UHktSimulationStashComponent>> RegisteredStashComponents;

	// Global Entity Manager
	FHktEntityManager EntityManager;

	// [Optimized] VM Pool for reusing VM instances
	TUniquePtr<FHktVMPool> VMPool;

	// Active VMs (now just pointers, owned by pool)
	TArray<FHktFlowVM*> ActiveVMs;

	// [Optimized] External -> Internal Handle Mapping only
	TMap<int32, FUnitHandle> ExternalToInternalMap;

	// 다음 외부 ID 생성용 카운터
	int32 NextExternalUnitId = 1;

	// --- Lockstep Event Processing ---
	
	// 마지막 처리 프레임 번호 추적
	int32 LastProcessedFrameNumber = 0;

	// --- Synchronization ---
	
	// Simulation 실행 상태 플래그 (Late Join 감지용)
	bool bSimulationRunning = false;
};

// Type alias for backward compatibility (deprecated)
using UHktSimulationSubsystem = UHktSimulationProcessSubsystem;
