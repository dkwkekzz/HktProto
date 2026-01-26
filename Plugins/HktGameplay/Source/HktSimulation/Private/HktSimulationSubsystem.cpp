#include "HktSimulationSubsystem.h"
#include "HktServiceSubsystem.h"
#include "HktSimulationStashComponent.h"
#include "Build/HktFlowBuildProcessor.h"
#include "Core/HktVMRuntime.h"
#include "Processors/HktWaitProcessor.h"
#include "Processors/HktExecuteProcessor.h"
#include "Processors/HktCleanupProcessor.h"
#include "Engine/World.h"

// 명령어 테이블 정의
HktOpsNew::FHktOpHandlerNew FHktExecuteDispatch::OpTable[static_cast<int32>(EHktOp::MAX)] = {};

// ============================================================================
// 라이프사이클
// ============================================================================

void UHktSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // 빌드 프로세서 생성
    BuildProcessor = MakeUnique<FHktFlowBuildProcessor>();
    BuildProcessor->SetDebugLog(bDebugLog);
    
    // 명령어 테이블 초기화
    FHktExecuteDispatch::Initialize();
    
    // 시뮬레이션 컨텍스트 초기화
    SimContext.JobQueue = &JobQueue;
    SimContext.WaitQueue = &WaitQueue;
    SimContext.StateStore = &StateStore;
    SimContext.ListStorage = &ListStorage;
    SimContext.UnrealWorld = GetWorld();
    SimContext.bDebugLog = bDebugLog;
    
    UE_LOG(LogTemp, Log, TEXT("[HktSimulation] Subsystem Initialized - JobQueue/WaitQueue Architecture Active"));
}

void UHktSimulationSubsystem::Deinitialize()
{
    JobQueue.Clear();
    WaitQueue.Clear();
    BuildProcessor.Reset();
    StateStore.Clear();
    ListStorage.Clear();
    SimulationStashComponents.Empty();
    PendingCompletedEvents.Empty();
    
    Super::Deinitialize();
}

void UHktSimulationSubsystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // 컨텍스트 업데이트
    SimContext.DeltaTime = DeltaTime;
    SimContext.FrameNumber = CompletedFrameNumber;
    SimContext.bDebugLog = bDebugLog;
    SimContext.CompletedEventIDs.Reset();
    
    // 빌드 프로세서 디버그 설정 동기화
    if (BuildProcessor.IsValid())
    {
        BuildProcessor->SetDebugLog(bDebugLog);
    }
    
    // 상태 이동 업데이트 (위치 등)
    StateStore.Tick(DeltaTime);
    
    // ========================================================================
    // Processor 순차 실행
    // ========================================================================
    
    // 1. 빌드: IntentEvent → VM 생성
    ProcessBuildVMs();
    
    // 2. 대기 조건 체크 (SOA 최적화)
    ProcessWaitConditions();
    
    // 3. 명령어 실행 (AOS 직관적)
    ProcessActiveVMs();
    
    // 4. 완료된 VM 정리
    ProcessCleanupFinished();
    
    // 5. 완료 이벤트 전파
    ProcessNotifyCompletions();
    
    CompletedFrameNumber++;
}

TStatId UHktSimulationSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UHktSimulationSubsystem, STATGROUP_Tickables);
}

UHktSimulationSubsystem* UHktSimulationSubsystem::Get(const UObject* WorldContextObject)
{
    if (!WorldContextObject) return nullptr;
    
    UWorld* World = WorldContextObject->GetWorld();
    if (!World) return nullptr;
    
    return World->GetSubsystem<UHktSimulationSubsystem>();
}

void UHktSimulationSubsystem::RegisterSimulationStashComponent(UHktSimulationStashComponent* Component)
{
    SimulationStashComponents.Add(Component);
}

void UHktSimulationSubsystem::UnregisterSimulationStashComponent(UHktSimulationStashComponent* Component)
{
    SimulationStashComponents.Remove(Component);
}

// ============================================================================
// 외부 API
// ============================================================================

FHktVMHandle UHktSimulationSubsystem::StartVM(const FGameplayTag& FlowTag, int32 EventID, int32 OwnerEntityID, int32 OwnerGeneration)
{
    if (!BuildProcessor.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[HktSimulation] BuildProcessor not initialized"));
        return FHktVMHandle::Invalid();
    }
    
    // 프로그램 빌드/캐시 조회
    const FHktProgram* Program = BuildProcessor->GetOrBuildProgram(FlowTag, nullptr);
    if (!Program || !Program->IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[HktSimulation] Failed to build program for: %s"), *FlowTag.ToString());
        return FHktVMHandle::Invalid();
    }
    
    // VM 시작
    FHktVMHandle Handle = JobQueue.StartVM(EventID, Program, OwnerEntityID, OwnerGeneration);
    
    if (Handle.IsValid() && bDebugLog)
    {
        UE_LOG(LogTemp, Log, TEXT("[HktSimulation] Started VM[%d] for Flow: %s, Event: %d, Owner: %d"), 
            Handle.SlotIndex, *FlowTag.ToString(), EventID, OwnerEntityID);
    }
    
    return Handle;
}

void UHktSimulationSubsystem::CancelVM(const FHktVMHandle& Handle)
{
    FHktVMRuntime* Runtime = JobQueue.Get(Handle);
    if (Runtime)
    {
        // WaitQueue에서 제거
        if (Runtime->WaitQueueIndex != INDEX_NONE)
        {
            WaitQueue.Remove(Runtime->WaitQueueIndex);
        }
        
        Runtime->State = EHktVMState::Finished;
        JobQueue.AddToFinished(Handle.SlotIndex);
        
        if (bDebugLog)
        {
            UE_LOG(LogTemp, Log, TEXT("[HktSimulation] Cancelled VM[%d]"), Handle.SlotIndex);
        }
    }
}

FHktVMHandle UHktSimulationSubsystem::GetVMHandleByEventID(int32 EventID) const
{
    FHktVMRuntime* Runtime = const_cast<FHktJobQueue&>(JobQueue).GetByEventID(EventID);
    if (Runtime)
    {
        return FHktVMHandle{ Runtime->SlotIndex, Runtime->Generation };
    }
    return FHktVMHandle::Invalid();
}

void UHktSimulationSubsystem::NotifyCollision(int32 EntityID, int32 Generation)
{
    SimContext.LastCollisionEntityID = EntityID;
    SimContext.LastCollisionGeneration = Generation;
}

// ============================================================================
// 내부 처리 (Processor 호출)
// ============================================================================

void UHktSimulationSubsystem::ProcessBuildVMs()
{
    if (!BuildProcessor.IsValid()) return;
    
    UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld());
    if (!Service) return;
    
    TScriptInterface<IHktIntentEventProvider> IntentEventProvider = Service->GetIntentEventProvider();
    if (!IntentEventProvider) return;
        
    TArray<FHktIntentEvent> IntentEvents;
    IntentEventProvider->PullIntentEvents(CompletedFrameNumber, IntentEvents);
    if (IntentEvents.IsEmpty()) return;

    for (const FHktIntentEvent& IntentEvent : IntentEvents)
    {
        // Flow 태그 조회 (EventTag를 Flow 태그로 사용)
        FGameplayTag FlowTag = IntentEvent.EventTag;
        if (!FlowTag.IsValid())
        {
            continue;
        }
        
        // 프로그램 빌드
        const FHktProgram* Program = BuildProcessor->GetOrBuildProgram(FlowTag, nullptr);
        if (!Program || !Program->IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("[HktSimulation] Failed to build program for intent: %s"), 
                *FlowTag.ToString());
            continue;
        }
        
        // VM 시작
        int32 EventID = IntentEvent.EventId;
        int32 OwnerID = IntentEvent.Subject.Value;
        int32 OwnerGen = 0;  // TODO: Subject에서 Generation 가져오기
        
        FHktVMHandle Handle = JobQueue.StartVM(EventID, Program, OwnerID, OwnerGen);
        
        if (Handle.IsValid() && bDebugLog)
        {
            UE_LOG(LogTemp, Log, TEXT("[HktSimulation] Built VM for Intent: %s, Event: %d"), 
                *FlowTag.ToString(), EventID);
        }
    }
}

void UHktSimulationSubsystem::ProcessWaitConditions()
{
    TArray<FHktWaitCompletion> Completions;
    FHktWaitProcessor::Process(SimContext, Completions);
    
    // 충돌 대기 완료 시 결과를 레지스터에 저장
    for (const FHktWaitCompletion& Completion : Completions)
    {
        if (Completion.Condition == EHktYieldCondition::Collision && 
            Completion.CollisionEntityID != INDEX_NONE)
        {
            FHktVMRuntime* Runtime = JobQueue.Get(
                FHktVMHandle{ Completion.RuntimeSlot, Completion.RuntimeGeneration });
            if (Runtime)
            {
                // TODO: 충돌 결과를 적절한 레지스터에 저장
                // 현재는 WaitCollision 명령어의 A 레지스터에 저장해야 하는데,
                // 해당 정보를 WaitQueue에 저장하지 않아서 어려움
            }
        }
    }
}

void UHktSimulationSubsystem::ProcessActiveVMs()
{
    FHktExecuteProcessor::Process(SimContext);
}

void UHktSimulationSubsystem::ProcessCleanupFinished()
{
    TArray<FHktCompletedEvent> CompletedEvents;
    FHktCleanupProcessor::Process(SimContext, CompletedEvents);
    
    // 완료 이벤트를 pending 목록에 추가 (다음 틱에 전파)
    PendingCompletedEvents.Append(CompletedEvents);
}

void UHktSimulationSubsystem::ProcessNotifyCompletions()
{
    if (PendingCompletedEvents.Num() == 0)
    {
        return;
    }
    
    // SimulationStashComponent에 전파
    TArray<UHktSimulationStashComponent*> ValidComponents;
    for (TObjectPtr<UHktSimulationStashComponent> Component : SimulationStashComponents)
    {
        if (Component && IsValid(Component))
        {
            ValidComponents.Add(Component);
        }
    }
    
    FHktCleanupProcessor::NotifyCompletions(PendingCompletedEvents, ValidComponents);
    
    PendingCompletedEvents.Empty();
}
