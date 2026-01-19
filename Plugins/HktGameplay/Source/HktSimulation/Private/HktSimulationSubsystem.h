// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HktEntityManager.h"
#include "HktFlowVM.h"
#include "IHktPlayerAttributeProvider.h"
#include "HktSimulationSubsystem.generated.h"

struct FHktIntentEvent;

// 로깅 카테고리 선언
DECLARE_LOG_CATEGORY_EXTERN(LogHktSimulation, Log, All);

/**
 * World Subsystem for managing HktSimulation
 * - IntentEvent를 받아 FlowVM을 통해 시뮬레이션을 실행
 * - Entity/Player Database를 관리
 * - IHktPlayerAttributeProvider 구현: Player 속성 변경을 외부에 제공
 * 
 * Sliding Window 방식:
 * - FetchNewEvents로 마지막 처리 프레임 이후의 이벤트만 조회
 * - 처리 성공 시 프레임 커서 업데이트
 * - 실패 시 커서 유지하여 다음 프레임에 재시도
 */
class FHktVMPool;

UCLASS()
class HKTSIMULATION_API UHktSimulationSubsystem : public UTickableWorldSubsystem, public IHktPlayerAttributeProvider
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

	// --- IHktPlayerAttributeProvider 구현 ---
	virtual FOnPlayerAttributeChanged& OnPlayerAttributeChanged() override;
	virtual bool GetPlayerAttributeSnapshot(const FHktPlayerHandle& PlayerHandle, FHktPlayerAttributeSnapshot& OutSnapshot) const override;
	virtual bool ConsumeChangedPlayers(TArray<FHktPlayerAttributeSnapshot>& OutSnapshots) override;
	virtual FHktPlayerHandle RegisterPlayer() override;
	virtual void UnregisterPlayer(const FHktPlayerHandle& PlayerHandle) override;

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
	
	/** 현재 커서(마지막 처리 프레임) 조회 */
	int32 GetLastProcessedFrame() const { return LastProcessedFrame; }
	
	/** 커서 강제 설정 (Late Join 시 사용) */
	void SetLastProcessedFrame(int32 NewFrame) { LastProcessedFrame = NewFrame; }

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
	/** FHktAttributeSet를 FHktPlayerAttributeSnapshot으로 변환 */
	void ConvertToSnapshot(int32 PlayerIndex, const FHktAttributeSet& Attrs, FHktPlayerAttributeSnapshot& OutSnapshot) const;

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

	// --- Sliding Window Cursor ---
	int32 LastProcessedFrame = 0;
	
	// --- Player Attribute Provider ---
	FOnPlayerAttributeChanged OnPlayerAttributeChangedDelegate;
};
