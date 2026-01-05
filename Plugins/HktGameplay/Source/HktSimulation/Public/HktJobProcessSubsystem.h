// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HktFlowInterfaces.h"
#include "IHktJobProvider.h"
#include "HktJobProcessSubsystem.generated.h"

/**
 * [Job Execution Result]
 * Result of processing a single job.
 */
UENUM(BlueprintType)
enum class EHktJobResult : uint8
{
	Success,
	Failed,
	Pending,    // Job still in progress
	Cancelled
};

/**
 * [Job Process Subsystem]
 * Processes jobs from the JobProvider (accessed via HktService).
 * 
 * This subsystem:
 * - Retrieves jobs from IHktJobProvider
 * - Executes jobs based on their Op type
 * - Manages job lifecycle (pending -> executing -> completed)
 * - Supports extensible job handlers via delegates
 * - Subscribes to flow changes from IHktJobProvider
 */
UCLASS()
class HKTSIMULATION_API UHktJobProcessSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

	// Helper to get the subsystem from a world context
	static UHktJobProcessSubsystem* Get(const UObject* WorldContextObject);

protected:
	/** Flow 변경 시 호출되는 핸들러 */
	void HandleFlowChanged(const FHktFlowChangedData& ChangedData);

private:
	/** JobProvider 구독을 위한 델리게이트 핸들 */
	FDelegateHandle FlowChangedHandle;

	/** Job 핸들러 맵 (확장을 위해 예약) */
	TMap<FGameplayTag, TFunction<EHktJobResult(int32)>> JobHandlers;

	/** 현재 실행 중인 Job Id 목록 */
	TSet<int32> ExecutingJobIds;
};

