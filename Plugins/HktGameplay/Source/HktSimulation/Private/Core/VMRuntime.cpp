#include "VMRuntime.h"
#include "VMProgram.h"

FString FVMRuntime::GetDebugString() const
{
    const TCHAR* StatusNames[] = {
        TEXT("Ready"), TEXT("Running"), TEXT("Yielded"), 
        TEXT("WaitingEvent"), TEXT("Completed"), TEXT("Failed")
    };
    
    return FString::Printf(
        TEXT("[VM] Tag=%s PC=%d Status=%s Self=%d Target=%d Spawned=%d"),
        Program ? *Program->Tag.ToString() : TEXT("null"),
        PC,
        StatusNames[static_cast<int32>(Status)],
        Registers[Reg::Self],
        Registers[Reg::Target],
        Registers[Reg::Spawned]
    );
}

// ============================================================================
// FVMRuntimePool
// ============================================================================

FVMRuntimePool::FVMRuntimePool()
{
    Statuses.SetNumZeroed(MaxVMs);
    PCs.SetNumZeroed(MaxVMs);
    WaitFrames.SetNumZeroed(MaxVMs);
    Generations.SetNumZeroed(MaxVMs);
    Runtimes.SetNum(MaxVMs);
    
    FreeSlots.Reserve(MaxVMs);
    for (int32 i = MaxVMs - 1; i >= 0; --i)
    {
        FreeSlots.Add(i);
        Statuses[i] = EVMStatus::Completed;
    }
}

FVMHandle FVMRuntimePool::Allocate()
{
    if (FreeSlots.Num() == 0)
        return FVMHandle::Invalid();
    
    uint32 Index = FreeSlots.Pop();
    
    FVMHandle Handle;
    Handle.Index = Index;
    Handle.Generation = Generations[Index];
    
    Statuses[Index] = EVMStatus::Ready;
    PCs[Index] = 0;
    WaitFrames[Index] = 0;
    
    FVMRuntime& Runtime = Runtimes[Index];
    Runtime.Program = nullptr;
    Runtime.Store = nullptr;
    Runtime.PC = 0;
    Runtime.Status = EVMStatus::Ready;
    Runtime.CreationFrame = 0;
    Runtime.WaitFrames = 0;
    Runtime.EventWait.Reset();
    Runtime.SpatialQuery.Reset();
    FMemory::Memzero(Runtime.Registers, sizeof(Runtime.Registers));
    
    return Handle;
}

void FVMRuntimePool::Free(FVMHandle Handle)
{
    if (!IsValid(Handle))
        return;
    
    uint32 Index = Handle.Index;
    Generations[Index]++;
    Statuses[Index] = EVMStatus::Completed;
    FreeSlots.Add(Index);
}

FVMRuntime* FVMRuntimePool::Get(FVMHandle Handle)
{
    if (!IsValid(Handle))
        return nullptr;
    return &Runtimes[Handle.Index];
}

const FVMRuntime* FVMRuntimePool::Get(FVMHandle Handle) const
{
    if (!IsValid(Handle))
        return nullptr;
    return &Runtimes[Handle.Index];
}

bool FVMRuntimePool::IsValid(FVMHandle Handle) const
{
    if (!Handle.IsValid() || Handle.Index >= static_cast<uint32>(MaxVMs))
        return false;
    return Generations[Handle.Index] == Handle.Generation;
}

int32 FVMRuntimePool::CountByStatus(EVMStatus Status) const
{
    int32 Count = 0;
    for (int32 i = 0; i < Runtimes.Num(); ++i)
    {
        if (Statuses[i] == Status)
            Count++;
    }
    return Count;
}

void FVMRuntimePool::Reset()
{
    FreeSlots.Reset();
    for (int32 i = MaxVMs - 1; i >= 0; --i)
    {
        FreeSlots.Add(i);
        Statuses[i] = EVMStatus::Completed;
        Generations[i]++;
    }
}