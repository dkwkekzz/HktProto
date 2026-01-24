#include "HktSimulationSubsystem.h"
#include "HktServiceSubsystem.h"
#include "Core/HktVM.h"
#include "Build/HktFlowBuildProcessor.h"
#include "Engine/World.h"

// ============================================================================
// 라이프사이클
// ============================================================================

void UHktSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // 빌드 프로세서 생성
    BuildProcessor = MakeUnique<FHktFlowBuildProcessor>();
    BuildProcessor->SetDebugLog(bDebugLog);
    
    // 월드 컨텍스트 초기화
    VMWorld.Attributes = &AttributeStore;
    VMWorld.Lists = &ListStorage;
    VMWorld.UnrealWorld = GetWorld();
    VMWorld.bDebugLog = bDebugLog;
    
    UE_LOG(LogTemp, Log, TEXT("[HktSimulation] Subsystem Initialized - DOD VM Active"));
}

void UHktSimulationSubsystem::Deinitialize()
{
    VMBatch.Clear();
    BuildProcessor.Reset();
    AttributeStore.Clear();
    ListStorage.Clear();
    
    Super::Deinitialize();
}

void UHktSimulationSubsystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // 월드 컨텍스트 업데이트
    VMWorld.DeltaTime = DeltaTime;
    VMWorld.bDebugLog = bDebugLog;
    
    // 빌드 프로세서 디버그 설정 동기화
    if (BuildProcessor.IsValid())
    {
        BuildProcessor->SetDebugLog(bDebugLog);
    }
    
    // 속성 이동 업데이트
    AttributeStore.TickMovement(DeltaTime);
    
    // VM 처리
    ProcessBuildVMs();
    ProcessActiveVMs(DeltaTime);
    ProcessCleanupFinishedVMs();
}

TStatId UHktSimulationSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UHktSimulationSubsystem, STATGROUP_Tickables);
}

// ============================================================================
// 외부 API
// ============================================================================

int32 UHktSimulationSubsystem::StartVM(const FGameplayTag& FlowTag, int32 OwnerEntityID, int32 OwnerGeneration)
{
    if (!BuildProcessor.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[HktSimulation] BuildProcessor not initialized"));
        return INDEX_NONE;
    }
    
    const FHktProgram* Program = BuildProcessor->GetOrBuildProgram(FlowTag, nullptr, &AttributeStore);
    if (!Program || !Program->IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[HktSimulation] Failed to build program for: %s"), *FlowTag.ToString());
        return INDEX_NONE;
    }
    
    int32 Slot = HktVMUtils::StartVM(VMBatch, Program, OwnerEntityID, OwnerGeneration);
    
    if (bDebugLog && Slot != INDEX_NONE)
    {
        UE_LOG(LogTemp, Log, TEXT("[HktSimulation] Started VM[%d] for Flow: %s, Owner: %d"), 
            Slot, *FlowTag.ToString(), OwnerEntityID);
    }
    
    return Slot;
}

// ============================================================================
// 내부 처리
// ============================================================================

void UHktSimulationSubsystem::ProcessBuildVMs()
{
    if (!BuildProcessor.IsValid()) return;
    
    UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld());
    if (!Service) return;
    
    for (TScriptInterface<IHktIntentEventProvider> Provider : Service->GetIntentEventProviders())
    {
        if (!Provider.IsValid()) continue;
        
        TArray<FHktIntentEventBatch> Batches = Provider->GetPendingBatches();
        for (const FHktIntentEventBatch& Batch : Batches)
        {
            for (const FHktIntentEvent& Event : Batch.Events)
            {
                // BuildProcessor를 통해 프로그램 가져오기/빌드
                const FHktProgram* Program = BuildProcessor->GetOrBuildProgram(
                    Event.EventTag, 
                    &Event, 
                    &AttributeStore);
                    
                if (!Program || !Program->IsValid())
                {
                    continue;
                }
                
                // VM 시작
                int32 OwnerID = Event.Subject.Value;
                int32 OwnerGen = 0;  // TODO: Subject에서 Generation 가져오기
                
                int32 Slot = HktVMUtils::StartVM(VMBatch, Program, OwnerID, OwnerGen);
                
                if (bDebugLog && Slot != INDEX_NONE)
                {
                    UE_LOG(LogTemp, Log, TEXT("[HktSimulation] VM[%d] started: %s (Entity %d)"), 
                        Slot, *Event.EventTag.ToString(), OwnerID);
                }
            }
        }
    }
}

void UHktSimulationSubsystem::ProcessActiveVMs(float DeltaTime)
{
    // 배치 단위로 실행
    FHktVM::TickBatch(VMBatch, VMWorld);
}

void UHktSimulationSubsystem::ProcessCleanupFinishedVMs()
{
    VMBatch.CleanupFinished();
}
