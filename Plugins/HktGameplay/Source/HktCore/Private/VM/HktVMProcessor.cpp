#include "HktVMProcessor.h"
#include "HktCoreInterfaces.h"
#include "HktVMInterpreter.h"
#include "HktVMStore.h"
#include "HktVMProgram.h"

FHktVMProcessor::~FHktVMProcessor()
{
    if (Interpreter)
    {
        delete Interpreter;
        Interpreter = nullptr;
    }
}

void FHktVMProcessor::Initialize(IHktStashInterface* InStash)
{
    Stash = InStash;
    
    Interpreter = new FHktVMInterpreter();
    Interpreter->Initialize(Stash);

    // RuntimePool은 인라인 멤버이므로 Reset으로 초기화
    RuntimePool.Reset();

    // Store 풀 초기화
    StorePool.SetNum(256);
    for (FHktVMStore& Store : StorePool)
    {
        Store.Stash = Stash;
    }
}

void FHktVMProcessor::Tick(int32 CurrentFrame, float DeltaSeconds)
{
    Build(CurrentFrame);
    Execute(DeltaSeconds);
    Cleanup(CurrentFrame);
}

void FHktVMProcessor::NotifyIntentEvent(const FHktIntentEvent& Event)
{
    PendingEvents.Add(Event);
}

// ============================================================================
// Event Notifications
// ============================================================================

void FHktVMProcessor::NotifyCollision(EntityId WatchedEntity, EntityId HitEntity)
{
    RuntimePool.ForEachActive([&](FHktVMHandle Handle, FHktVMRuntime& Runtime)
    {
        if (Runtime.Status == EVMStatus::WaitingEvent &&
            Runtime.EventWait.Type == EWaitEventType::Collision &&
            Runtime.EventWait.WatchedEntity == WatchedEntity)
        {
            Interpreter->NotifyCollision(Runtime, HitEntity);
        }
    });
}

void FHktVMProcessor::NotifyAnimEnd(EntityId Entity)
{
    RuntimePool.ForEachActive([&](FHktVMHandle Handle, FHktVMRuntime& Runtime)
    {
        if (Runtime.Status == EVMStatus::WaitingEvent &&
            Runtime.EventWait.Type == EWaitEventType::AnimationEnd &&
            Runtime.EventWait.WatchedEntity == Entity)
        {
            Interpreter->NotifyAnimEnd(Runtime);
        }
    });
}

void FHktVMProcessor::NotifyMoveEnd(EntityId Entity)
{
    RuntimePool.ForEachActive([&](FHktVMHandle Handle, FHktVMRuntime& Runtime)
    {
        if (Runtime.Status == EVMStatus::WaitingEvent &&
            Runtime.EventWait.Type == EWaitEventType::MovementEnd &&
            Runtime.EventWait.WatchedEntity == Entity)
        {
            Interpreter->NotifyMoveEnd(Runtime);
        }
    });
}

// ============================================================================
// Phase 1: Build
// ============================================================================

void FHktVMProcessor::Build(int32 CurrentFrame)
{
    TArray<FHktIntentEvent> Events = PullIntentEvents();
    Events.Sort();
    
    for (const FHktIntentEvent& Event : Events)
    {
        // 1. 첨부된 스냅샷 먼저 적용
        ApplyAttachedSnapshots(Event);
        
        // 2. VM 생성
        TOptional<FHktVMHandle> Handle = TryCreateVM(Event, CurrentFrame);
        if (Handle.IsSet())
        {
            PendingVMs.Add(Handle.GetValue());
        }
    }
    
    ActiveVMs.Append(PendingVMs);
    PendingVMs.Reset();
    
    // Yielded VM 재활성화
    RuntimePool.ForEachActive([](FHktVMHandle Handle, FHktVMRuntime& Runtime)
    {
        if (Runtime.Status == EVMStatus::Yielded)
        {
            if (Runtime.WaitFrames <= 0)
            {
                Runtime.Status = EVMStatus::Ready;
            }
            else
            {
                Runtime.WaitFrames--;
            }
        }
    });
}

TArray<FHktIntentEvent> FHktVMProcessor::PullIntentEvents()
{
    TArray<FHktIntentEvent> Result = MoveTemp(PendingEvents);
    PendingEvents.Reset();
    return Result;
}

void FHktVMProcessor::ApplyAttachedSnapshots(const FHktIntentEvent& Event)
{
    if (!Stash || Event.AttachedSnapshots.Num() == 0)
        return;
    
    for (const FHktEntitySnapshot& Snapshot : Event.AttachedSnapshots)
    {
        if (!Snapshot.IsValid())
            continue;
        
        FHktEntityId E = Snapshot.GetEntityId();
        
        // 이미 알고 있는 엔티티면 스킵
        if (Stash->IsValidEntity(E))
            continue;
        
        // 스냅샷 적용: 엔티티 생성 후 속성 설정
        // 참고: 여기서는 AllocateEntity 대신 직접 SetProperty
        // VisibleStash의 경우 알려지지 않은 엔티티도 SetProperty 가능해야 함
        for (int32 PropId = 0; PropId < Snapshot.Properties.Num(); ++PropId)
        {
            Stash->SetProperty(E, static_cast<uint16>(PropId), Snapshot.Properties[PropId]);
        }
        
        UE_LOG(LogTemp, Log, TEXT("[VMProcessor] Applied attached snapshot for Entity %u"), E.RawValue);
    }
}

TOptional<FHktVMHandle> FHktVMProcessor::TryCreateVM(const FHktIntentEvent& Event, int32 CurrentFrame)
{
    if (!Stash || !Stash->IsValidEntity(Event.SourceEntity))
    {
        UE_LOG(LogTemp, Warning, TEXT("VM creation failed: SourceEntity %u not valid"), (int32)Event.SourceEntity);
        return {};
    }
    
    const FHktVMProgram* Program = FHktVMProgramRegistry::Get().FindProgram(Event.EventTag);
    if (!Program)
    {
        UE_LOG(LogTemp, Warning, TEXT("VM creation failed: No program for %s"), *Event.EventTag.ToString());
        return {};
    }
    
    FHktVMHandle Handle = RuntimePool.Allocate();
    if (!Handle.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("VM creation failed: Pool exhausted"));
        return {};
    }
    
    FHktVMRuntime* Runtime = RuntimePool.Get(Handle);
    check(Runtime);
    
    // Store 할당
    FHktVMStore& Store = StorePool[Handle.Index];
    Store.Stash = Stash;
    Store.SourceEntity = Event.SourceEntity;
    Store.TargetEntity = Event.TargetEntity;
    Store.ClearPendingWrites();
    Store.LocalCache.Reset();
    
    // Runtime 초기화
    Runtime->Program = Program;
    Runtime->Store = &Store;
    Runtime->PC = 0;
    Runtime->Status = EVMStatus::Ready;
    Runtime->CreationFrame = CurrentFrame;
    Runtime->WaitFrames = 0;
    Runtime->EventWait.Reset();
    Runtime->SpatialQuery.Reset();
    FMemory::Memzero(Runtime->Registers, sizeof(Runtime->Registers));
    
    Runtime->SetRegEntity(Reg::Self, Event.SourceEntity);
    Runtime->SetRegEntity(Reg::Target, Event.TargetEntity);
    
    // Payload에서 파라미터 추출 (int32 배열로 해석)
    const int32* PayloadParams = reinterpret_cast<const int32*>(Event.Payload.GetData());
    const int32 NumParams = FMath::Min((int32)(Event.Payload.Num() / sizeof(int32)), 4);
    for (int32 i = 0; i < NumParams; ++i)
    {
        Store.Write(PropertyId::Param0 + i, PayloadParams[i]);
    }
    
    // 타겟 위치 설정 (Event.Location 사용)
    Store.Write(PropertyId::TargetPosX, FMath::RoundToInt(Event.Location.X));
    Store.Write(PropertyId::TargetPosY, FMath::RoundToInt(Event.Location.Y));
    Store.Write(PropertyId::TargetPosZ, FMath::RoundToInt(Event.Location.Z));
    
    UE_LOG(LogTemp, Log, TEXT("VM created: %s for Entity %u"), *Event.EventTag.ToString(), (int32)Event.SourceEntity);
    
    return Handle;
}

// ============================================================================
// Phase 2: Execute
// ============================================================================

void FHktVMProcessor::Execute(float DeltaSeconds)
{
    RuntimePool.ForEachActive([&](FHktVMHandle Handle, FHktVMRuntime& Runtime)
    {
        if (Runtime.Status == EVMStatus::WaitingEvent &&
            Runtime.EventWait.Type == EWaitEventType::Timer)
        {
            Interpreter->UpdateTimer(Runtime, DeltaSeconds);
        }
    });
    
    for (int32 i = ActiveVMs.Num() - 1; i >= 0; --i)
    {
        FHktVMHandle Handle = ActiveVMs[i];
        EVMStatus Status = ExecuteUntilYield(Handle, DeltaSeconds);
        
        if (Status == EVMStatus::Completed || Status == EVMStatus::Failed)
        {
            CompletedVMs.Add(Handle);
            ActiveVMs.RemoveAtSwap(i);
        }
    }
}

EVMStatus FHktVMProcessor::ExecuteUntilYield(FHktVMHandle Handle, float DeltaSeconds)
{
    FHktVMRuntime* Runtime = RuntimePool.Get(Handle);
    if (!Runtime) return EVMStatus::Failed;
    
    if (!Runtime->IsRunnable())
        return Runtime->Status;
    
    Runtime->Status = EVMStatus::Running;
    EVMStatus Result = Interpreter->Execute(*Runtime);
    Runtime->Status = Result;
    
    return Result;
}

// ============================================================================
// Phase 3: Cleanup
// ============================================================================

void FHktVMProcessor::Cleanup(int32 CurrentFrame)
{
    for (FHktVMHandle Handle : CompletedVMs)
    {
        ApplyStoreChanges(Handle);
        FinalizeVM(Handle);
    }
    CompletedVMs.Reset();
}

void FHktVMProcessor::ApplyStoreChanges(FHktVMHandle Handle)
{
    FHktVMRuntime* Runtime = RuntimePool.Get(Handle);
    if (!Runtime || !Runtime->Store || !Stash) return;
    
    for (const FHktVMStore::FPendingWrite& W : Runtime->Store->PendingWrites)
    {
        Stash->SetProperty(W.Entity, W.PropertyId, W.Value);
    }
    Runtime->Store->ClearPendingWrites();
}

void FHktVMProcessor::FinalizeVM(FHktVMHandle Handle)
{
    FHktVMRuntime* Runtime = RuntimePool.Get(Handle);
    if (Runtime)
    {
        UE_LOG(LogTemp, Log, TEXT("VM finalized: %s"), 
            Runtime->Program ? *Runtime->Program->Tag.ToString() : TEXT("unknown"));
        
        if (Runtime->Store)
        {
            Runtime->Store->Reset();
        }
    }
    RuntimePool.Free(Handle);
}