#include "VMProcessor.h"
#include "VMInterpreter.h"

void FVMProcessor::Initialize(UWorld* InWorld, FStashBase* InStash)
{
    World = InWorld;
    Stash = InStash;
    
    Interpreter = new FVMInterpreter();
    Interpreter->Initialize();
    
    // Store 풀 초기화
    StorePool.SetNum(256);
}

void FVMProcessor::Tick(int32 CurrentFrame, float DeltaSeconds)
{
    Build(CurrentFrame);
    Execute(DeltaSeconds);
    Cleanup(CurrentFrame);
}

void FVMProcessor::QueueIntentEvent(const FIntentEvent& Event)
{
    PendingEvents.Add(Event);
}

// ============================================================================
// Event Notifications
// ============================================================================

void FVMProcessor::NotifyCollision(EntityId WatchedEntity, EntityId HitEntity)
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

void FVMProcessor::NotifyAnimEnd(EntityId Entity)
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

void FVMProcessor::NotifyMoveEnd(EntityId Entity)
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

void FVMProcessor::Build(int32 CurrentFrame)
{
    TArray<FIntentEvent> Events = PullIntentEvents();
    Events.Sort();
    
    for (const FIntentEvent& Event : Events)
    {
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

TArray<FIntentEvent> FVMProcessor::PullIntentEvents()
{
    TArray<FIntentEvent> Result = MoveTemp(PendingEvents);
    PendingEvents.Reset();
    return Result;
}

TOptional<FVMHandle> FVMProcessor::TryCreateVM(const FIntentEvent& Event, int32 CurrentFrame)
{
    if (!ValidateStoreFrame(Event.SourceEntity, CurrentFrame))
    {
        UE_LOG(LogTemp, Warning, TEXT("VM creation failed: Store validation failed"));
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
    Store.SourceEntity = Event.SourceEntity;
    Store.TargetEntity = Event.TargetEntity;
    Store.ClearPendingWrites();
    
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
    
    // Self 레지스터 초기화
    Runtime->SetRegEntity(Reg::Self, Event.SourceEntity);
    Runtime->SetRegEntity(Reg::Target, Event.TargetEntity);
    
    // 이벤트 파라미터 복사
    for (int32 i = 0; i < Event.Parameters.Num() && i < 4; ++i)
    {
        Store.Write(PropertyId::Param0 + i, Event.Parameters[i]);
    }
    
    // 타겟 위치 복사
    Store.Write(PropertyId::TargetPosX, FMath::RoundToInt(Event.TargetLocation.X));
    Store.Write(PropertyId::TargetPosY, FMath::RoundToInt(Event.TargetLocation.Y));
    Store.Write(PropertyId::TargetPosZ, FMath::RoundToInt(Event.TargetLocation.Z));
    
    UE_LOG(LogTemp, Log, TEXT("VM created: %s for Entity %u"), *Event.EventTag.ToString(), Event.SourceEntity);
    
    return Handle;
}

bool FVMProcessor::ValidateStoreFrame(EntityId Entity, int32 CurrentFrame) const
{
    if (!Stash) return false;
    return Stash->ValidateEntityFrame(Entity, CurrentFrame);
}

// ============================================================================
// Phase 2: Execute
// ============================================================================

void FVMProcessor::Execute(float DeltaSeconds)
{
    // Timer 업데이트
    RuntimePool.ForEachActive([&](FVMHandle Handle, FVMRuntime& Runtime)
    {
        if (Runtime.Status == EVMStatus::WaitingEvent &&
            Runtime.EventWait.Type == EWaitEventType::Timer)
        {
            Interpreter->UpdateTimer(Runtime, DeltaSeconds);
        }
    });
    
    // 실행
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

EVMStatus FVMProcessor::ExecuteUntilYield(FVMHandle Handle, float DeltaSeconds)
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

void FVMProcessor::Cleanup(int32 CurrentFrame)
{
    // 완료된 VM만 Stash에 동기화
    // 활성 VM들의 PendingWrites는 유지 (VM 완료 시에만 적용)
    for (FVMHandle Handle : CompletedVMs)
    {
        ApplyStoreChanges(Handle);
        FinalizeVM(Handle);
    }
    CompletedVMs.Reset();
    
    // 참고: 활성 VM들의 PendingWrites는 적용하지 않음
    // VM이 완료되어야만 영구적 속성이 Stash에 반영됨
    // 이는 결정론적 실행과 롤백을 위해 필수적임
    
    if (Stash)
    {
        Stash->MarkFrameCompleted(CurrentFrame);
    }
}

void FVMProcessor::ApplyStoreChanges(FVMHandle Handle)
{
    FVMRuntime* Runtime = RuntimePool.Get(Handle);
    if (!Runtime || !Runtime->Store || !Stash) return;
    
    Stash->ApplyWrites(Runtime->Store->PendingWrites);
    Runtime->Store->ClearPendingWrites();
}

void FVMProcessor::FinalizeVM(FVMHandle Handle)
{
    FVMRuntime* Runtime = RuntimePool.Get(Handle);
    if (Runtime)
    {
        UE_LOG(LogTemp, Log, TEXT("VM finalized: %s"), 
            Runtime->Program ? *Runtime->Program->Tag.ToString() : TEXT("unknown"));
        
        // Store 리셋 (로컬 캐시 정리)
        if (Runtime->Store)
        {
            Runtime->Store->Reset();
        }
    }
    RuntimePool.Free(Handle);
}