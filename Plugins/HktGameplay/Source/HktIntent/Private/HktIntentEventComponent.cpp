// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktIntentEventComponent.h"
#include "HktIntentGameMode.h"
#include "HktServiceSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

UHktIntentEventComponent::UHktIntentEventComponent()
{
	SetIsReplicatedByDefault(true);
}

void UHktIntentEventComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld()))
	{
		SimulationProvider = Service->GetSimulationProvider();
	}

	if (AHktIntentGameMode* GameMode = AHktIntentGameMode::Get(GetWorld()))
	{
		GameMode->RegisterIntentEventComponent(this);
	}
}

void UHktIntentEventComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (AHktIntentGameMode* GameMode = AHktIntentGameMode::Get(GetWorld()))
	{
		GameMode->UnregisterIntentEventComponent(this);
	}
}

void UHktIntentEventComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Late-Join 클라이언트를 위한 Replication 설정
	DOREPLIFETIME_CONDITION(UHktIntentEventComponent, ProcessingIntentEventBatch, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(UHktIntentEventComponent, HktSimulationModel, COND_InitialOnly);
}

// ============================================================================
// Step 1: Client -> Server (Intent Submission)
// ============================================================================

void UHktIntentEventComponent::NotifyIntent(const FHktIntentEvent& IntentEvent)
{
	// 클라이언트에서 Server RPC 호출
	Server_NotifyIntent(IntentEvent);
}

void UHktIntentEventComponent::Server_NotifyIntent_Implementation(FHktIntentEvent IntentEvent)
{
	// 서버에서만 PendingIntentEvents 배열에 의도를 추가합니다.
	PendingIntentEvents.Add(MoveTemp(IntentEvent));
	
	UE_LOG(LogTemp, Verbose, TEXT("[HktIntentEventComponent] Server received and queued intent: Tag=%s"), *IntentEvent.EventTag.ToString());
}

bool UHktIntentEventComponent::Server_NotifyIntent_Validate(FHktIntentEvent IntentEvent)
{
	return true;
}

// ============================================================================
// Step 2: Server -> Client (Batch Distribution & Server Simulation)
// ============================================================================

void UHktIntentEventComponent::NotifyIntentBatch(int32 FrameNumber)
{
	if (PendingIntentEvents.Num() == 0)
	{
		return;
	}

	// 배치 생성 및 이벤트 이동
	FHktIntentEventBatch Batch(FrameNumber);
	Batch.Events = MoveTemp(PendingIntentEvents);
	PendingIntentEvents.Reset();

	// Late-Join용 배치 저장
	ProcessingIntentEventBatch = Batch;

	// 서버에서 시뮬레이션 실행 및 결과 저장
	if (SimulationProvider)
	{
		SimulationProvider->ProcessSimulation(Batch);
		UE_LOG(LogTemp, Log, TEXT("[HktIntentEventComponent] Server processed simulation for frame %d."), FrameNumber);
	}
}

void UHktIntentEventComponent::Server_NotifyCompletedSimulation(const FHktSimulationResult& SimulationResult)
{
	HktSimulationModel = SimulationResult;
	UE_LOG(LogTemp, Log, TEXT("[HktIntentEventComponent] Server received simulation result for frame %d."), SimulationResult.ProcessedFrameNumber);
}

// ============================================================================
// Step 4: Late-Join Synchronization
// ============================================================================

void UHktIntentEventComponent::OnRep_ProcessingIntentBatch()
{
	UE_LOG(LogTemp, Log, TEXT("[HktIntentEventComponent] Late-join client received initial processing batch for frame %d."), ProcessingIntentEventBatch.FrameNumber);
	// 필요 시, 여기서 멈춘 시뮬레이션을 재개하는 로직 추가
}

void UHktIntentEventComponent::OnRep_HktSimulationModel()
{
	UE_LOG(LogTemp, Log, TEXT("[HktIntentEventComponent] Late-join client received initial simulation model for frame %d."), HktSimulationModel.ProcessedFrameNumber);
	// 여기서 이 모델을 기반으로 상태를 복원하는 로직 추가
}
