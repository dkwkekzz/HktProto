#include "HktVMProcessor.h"
#include "VMInterpreter.h"

FHktVMProcessor::~FHktVMProcessor()
{
    if (Interpreter)
    {
        delete Interpreter;
        Interpreter = nullptr;
    }
}

void FHktVMProcessor::Initialize(UWorld* InWorld, IStashInterface* InStash)
{
    World = InWorld;
    Stash = InStash;
    
    Interpreter = new FVMInterpreter();
    Interpreter->Initialize(World, Stash);
    
    // Store 풀 초기화
    StorePool.SetNum(256);
    for (FVMStore& Store : StorePool)
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

void FHktVMProcessor::QueueIntentEvent(const FHktIntentEvent& Event)
{
    PendingEvents.Add(Event);
}

// ============================================================================
// Event Notifications
// ============================================================================

void FHktVMProcessor::NotifyCollision(EntityId WatchedEntity, EntityId HitEntity)
{
    RuntimePool.ForEachActive([&](FVMHandle Handle, FVMRuntime& Runtime)
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
    RuntimePool.ForEachActive([&](FVMHandle Handle, FVMRuntime& Runtime)
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
    RuntimePool.ForEachActive([&](FVMHandle Handle, FVMRuntime& Runtime)
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
        TOptional<FVMHandle> Handle = TryCreateVM(Event, CurrentFrame);
        if (Handle.IsSet())
        {
            PendingVMs.Add(Handle.GetValue());
        }
    }
    
    ActiveVMs.Append(PendingVMs);
    PendingVMs.Reset();
    
    // Yielded VM 재활성화
    RuntimePool.ForEachActive([](FVMHandle Handle, FVMRuntime& Runtime)
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
        
        UE_LOG(LogTemp, Log, TEXT("[VMProcessor] Applied attached snapshot for Entity %u"), E);
    }
}

TOptional<FVMHandle> FHktVMProcessor::TryCreateVM(const FHktIntentEvent& Event, int32 CurrentFrame)
{
    if (!Stash || !Stash->IsValidEntity(Event.SubjectEntityId))
    {
        UE_LOG(LogTemp, Warning, TEXT("VM creation failed: SubjectEntityId %u not valid"), Event.SubjectEntityId);
        return {};
    }
    
    const FVMProgram* Program = FVMProgramRegistry::Get().FindProgram(Event.EventTag);
    if (!Program)
    {
        UE_LOG(LogTemp, Warning, TEXT("VM creation failed: No program for %s"), *Event.EventTag.ToString());
        return {};
    }
    
    FVMHandle Handle = RuntimePool.Allocate();
    if (!Handle.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("VM creation failed: Pool exhausted"));
        return {};
    }
    
    FVMRuntime* Runtime = RuntimePool.Get(Handle);
    check(Runtime);
    
    // Store 할당
    FVMStore& Store = StorePool[Handle.Index];
    Store.Stash = Stash;
    Store.SourceEntity = Event.SubjectEntityId;
    Store.TargetEntity = Event.TargetEntityId;
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
    
    Runtime->SetRegEntity(Reg::Self, Event.SubjectEntityId);
    Runtime->SetRegEntity(Reg::Target, Event.TargetEntityId);
    
    for (int32 i = 0; i < Event.Parameters.Num() && i < 4; ++i)
    {
        Store.Write(PropertyId::Param0 + i, Event.Parameters[i]);
    }
    
    Store.Write(PropertyId::TargetPosX, FMath::RoundToInt(Event.TargetLocation.X));
    Store.Write(PropertyId::TargetPosY, FMath::RoundToInt(Event.TargetLocation.Y));
    Store.Write(PropertyId::TargetPosZ, FMath::RoundToInt(Event.TargetLocation.Z));
    
    UE_LOG(LogTemp, Log, TEXT("VM created: %s for Entity %u"), *Event.EventTag.ToString(), Event.SubjectEntityId);
    
    return Handle;
}

// ============================================================================
// Phase 2: Execute
// ============================================================================

void FHktVMProcessor::Execute(float DeltaSeconds)
{
    RuntimePool.ForEachActive([&](FVMHandle Handle, FVMRuntime& Runtime)
    {
        if (Runtime.Status == EVMStatus::WaitingEvent &&
            Runtime.EventWait.Type == EWaitEventType::Timer)
        {
            Interpreter->UpdateTimer(Runtime, DeltaSeconds);
        }
    });
    
    for (int32 i = ActiveVMs.Num() - 1; i >= 0; --i)
    {
        FVMHandle Handle = ActiveVMs[i];
        EVMStatus Status = ExecuteUntilYield(Handle, DeltaSeconds);
        
        if (Status == EVMStatus::Completed || Status == EVMStatus::Failed)
        {
            CompletedVMs.Add(Handle);
            ActiveVMs.RemoveAtSwap(i);
        }
    }
}

EVMStatus FHktVMProcessor::ExecuteUntilYield(FVMHandle Handle, float DeltaSeconds)
{
    FVMRuntime* Runtime = RuntimePool.Get(Handle);
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
    for (FVMHandle Handle : CompletedVMs)
    {
        ApplyStoreChanges(Handle);
        FinalizeVM(Handle);
    }
    CompletedVMs.Reset();
}

void FHktVMProcessor::ApplyStoreChanges(FVMHandle Handle)
{
    FVMRuntime* Runtime = RuntimePool.Get(Handle);
    if (!Runtime || !Runtime->Store || !Stash) return;
    
    for (const FVMStore::FPendingWrite& W : Runtime->Store->PendingWrites)
    {
        Stash->SetProperty(W.Entity, W.PropertyId, W.Value);
    }
    Runtime->Store->ClearPendingWrites();
}

void FHktVMProcessor::FinalizeVM(FVMHandle Handle)
{
    FVMRuntime* Runtime = RuntimePool.Get(Handle);
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