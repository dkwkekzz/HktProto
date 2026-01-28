#include "HktMasterStashComponent.h"

// ============================================================================
// UHktMasterStashComponent
// ============================================================================

UHktMasterStashComponent::UHktMasterStashComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    Properties.SetNum(MaxProperties);
    for (int32 i = 0; i < MaxProperties; ++i)
    {
        Properties[i].SetNumZeroed(MaxEntities);
    }
    
    ValidEntities.Init(false, MaxEntities);
    EntityCreationFrame.SetNumZeroed(MaxEntities);
}

// ============================================================================
// Entity Management
// ============================================================================

FHktEntityId UHktMasterStashComponent::AllocateEntity()
{
    FHktEntityId Id;
    
    if (FreeList.Num() > 0)
    {
        Id = FreeList.Pop();
    }
    else if (NextFHktEntityId < MaxEntities)
    {
        Id = NextFHktEntityId++;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[MasterStash] Entity limit reached!"));
        return InvalidFHktEntityId;
    }
    
    ValidEntities[Id] = true;
    EntityCreationFrame[Id] = CompletedFrameNumber;
    
    // 속성 초기화
    for (int32 PropId = 0; PropId < MaxProperties; ++PropId)
    {
        Properties[PropId][Id] = 0;
    }
    
    MarkDirty(Id);
    
    UE_LOG(LogTemp, Verbose, TEXT("[MasterStash] Entity %u allocated"), Id);
    return Id;
}

void UHktMasterStashComponent::FreeEntity(FHktEntityId Entity)
{
    if (Entity < MaxEntities && ValidEntities[Entity])
    {
        ValidEntities[Entity] = false;
        FreeList.Add(Entity);
        MarkDirty(Entity);
        
        UE_LOG(LogTemp, Verbose, TEXT("[MasterStash] Entity %u freed"), Entity);
    }
}

bool UHktMasterStashComponent::IsValidEntity(FHktEntityId Entity) const
{
    return Entity < MaxEntities && ValidEntities[Entity];
}

int32 UHktMasterStashComponent::GetEntityCount() const
{
    int32 Count = 0;
    for (int32 i = 0; i < MaxEntities; ++i)
    {
        if (ValidEntities[i])
            Count++;
    }
    return Count;
}

// ============================================================================
// Property Access
// ============================================================================

int32 UHktMasterStashComponent::GetProperty(FHktEntityId Entity, uint16 PropertyId) const
{
    if (!IsValidEntity(Entity) || PropertyId >= MaxProperties)
        return 0;
    return Properties[PropertyId][Entity];
}

void UHktMasterStashComponent::SetProperty(FHktEntityId Entity, uint16 PropertyId, int32 Value)
{
    if (IsValidEntity(Entity) && PropertyId < MaxProperties)
    {
        if (Properties[PropertyId][Entity] != Value)
        {
            Properties[PropertyId][Entity] = Value;
            MarkDirty(Entity);
        }
    }
}

void UHktMasterStashComponent::ApplyWrites(const TArray<FPendingWrite>& Writes)
{
    for (const auto& W : Writes)
    {
        SetProperty(W.Entity, W.PropertyId, W.Value);
    }
}

// ============================================================================
// Frame Management
// ============================================================================

void UHktMasterStashComponent::MarkFrameCompleted(int32 FrameNumber)
{
    CompletedFrameNumber = FrameNumber;
}

bool UHktMasterStashComponent::ValidateEntityFrame(FHktEntityId Entity, int32 FrameNumber) const
{
    if (!IsValidEntity(Entity))
        return false;
    return EntityCreationFrame[Entity] <= FrameNumber;
}

// ============================================================================
// Snapshot & Delta
// ============================================================================

FHktEntitySnapshot UHktMasterStashComponent::CreateEntitySnapshot(FHktEntityId Entity) const
{
    FHktEntitySnapshot Snapshot;
    
    if (!IsValidEntity(Entity))
        return Snapshot;
    
    Snapshot.EntityId = Entity;
    Snapshot.Properties.SetNum(MaxProperties);
    
    for (int32 PropId = 0; PropId < MaxProperties; ++PropId)
    {
        Snapshot.Properties[PropId] = Properties[PropId][Entity];
    }
    
    return Snapshot;
}

TArray<FHktEntitySnapshot> UHktMasterStashComponent::CreateSnapshots(const TArray<FHktEntityId>& Entities) const
{
    TArray<FHktEntitySnapshot> Snapshots;
    Snapshots.Reserve(Entities.Num());
    
    for (FHktEntityId E : Entities)
    {
        FHktEntitySnapshot Snap = CreateEntitySnapshot(E);
        if (Snap.IsValid())
        {
            Snapshots.Add(MoveTemp(Snap));
        }
    }
    
    return Snapshots;
}

TArray<uint8> UHktMasterStashComponent::SerializeFullState() const
{
    TArray<uint8> Data;
    FMemoryWriter Writer(Data);
    
    Writer << const_cast<int32&>(CompletedFrameNumber);
    Writer << const_cast<FHktEntityId&>(NextFHktEntityId);
    
    // Valid entities
    int32 NumValid = GetEntityCount();
    Writer << NumValid;
    
    for (FHktEntityId E = 0; E < MaxEntities; ++E)
    {
        if (ValidEntities[E])
        {
            Writer << const_cast<FHktEntityId&>(E);
            for (int32 PropId = 0; PropId < MaxProperties; ++PropId)
            {
                Writer << const_cast<int32&>(Properties[PropId][E]);
            }
        }
    }
    
    return Data;
}

void UHktMasterStashComponent::DeserializeFullState(const TArray<uint8>& Data)
{
    if (Data.Num() == 0)
        return;
    
    FMemoryReader Reader(Data);
    
    Reader << CompletedFrameNumber;
    Reader << NextFHktEntityId;
    
    // Clear all
    ValidEntities.Init(false, MaxEntities);
    FreeList.Reset();
    
    int32 NumValid = 0;
    Reader << NumValid;
    
    for (int32 i = 0; i < NumValid; ++i)
    {
        FHktEntityId E;
        Reader << E;
        
        ValidEntities[E] = true;
        for (int32 PropId = 0; PropId < MaxProperties; ++PropId)
        {
            Reader << Properties[PropId][E];
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("[MasterStash] Deserialized: Frame=%d, Entities=%d"), 
        CompletedFrameNumber, NumValid);
}

// ============================================================================
// Checksum
// ============================================================================

uint32 UHktMasterStashComponent::CalculateChecksum() const
{
    uint32 Checksum = 0;
    
    ForEachEntity([&](FHktEntityId E)
    {
        for (int32 PropId = 0; PropId < MaxProperties; ++PropId)
        {
            Checksum ^= Properties[PropId][E];
            Checksum = (Checksum << 1) | (Checksum >> 31);
        }
        Checksum ^= E;
    });
    
    Checksum ^= CompletedFrameNumber;
    return Checksum;
}

uint32 UHktMasterStashComponent::CalculatePartialChecksum(const TArray<FHktEntityId>& Entities) const
{
    uint32 Checksum = 0;
    
    for (FHktEntityId E : Entities)
    {
        if (!IsValidEntity(E))
            continue;
        
        for (int32 PropId = 0; PropId < MaxProperties; ++PropId)
        {
            Checksum ^= Properties[PropId][E];
            Checksum = (Checksum << 1) | (Checksum >> 31);
        }
        Checksum ^= E;
    }
    
    return Checksum;
}

// ============================================================================
// Change Tracking
// ============================================================================

void UHktMasterStashComponent::MarkDirty(FHktEntityId Entity)
{
    DirtyEntities.Add(Entity);
}

void UHktMasterStashComponent::ClearDirtyFlags()
{
    DirtyEntities.Reset();
}