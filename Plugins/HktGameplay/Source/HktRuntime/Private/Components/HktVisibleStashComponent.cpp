#include "HktVisibleStashComponent.h"

UHktVisibleStashComponent::UHktVisibleStashComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    Properties.SetNum(MaxProperties);
    for (int32 i = 0; i < MaxProperties; ++i)
    {
        Properties[i].SetNumZeroed(MaxEntities);
    }
    
    ValidEntities.Init(false, MaxEntities);
}

// ============================================================================
// Entity Management
// ============================================================================

FHktEntityId UHktVisibleStashComponent::AllocateEntity()
{
    FHktEntityId Id;
    
    if (FreeList.Num() > 0)
    {
        Id = FreeList.Pop();
    }
    else if (NextEntityId < MaxEntities)
    {
        Id = NextEntityId++;
    }
    else
    {
        return InvalidEntityId;
    }
    
    ValidEntities[Id] = true;
    
    for (int32 PropId = 0; PropId < MaxProperties; ++PropId)
    {
        Properties[PropId][Id] = 0;
    }
    
    return Id;
}

void UHktVisibleStashComponent::FreeEntity(FHktEntityId Entity)
{
    if (Entity < MaxEntities && ValidEntities[Entity])
    {
        ValidEntities[Entity] = false;
        FreeList.Add(Entity);
    }
}

bool UHktVisibleStashComponent::IsValidEntity(FHktEntityId Entity) const
{
    return Entity < MaxEntities && ValidEntities[Entity];
}

int32 UHktVisibleStashComponent::GetEntityCount() const
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

int32 UHktVisibleStashComponent::GetProperty(FHktEntityId Entity, uint16 PropertyId) const
{
    if (!IsValidEntity(Entity) || PropertyId >= MaxProperties)
        return 0;
    return Properties[PropertyId][Entity];
}

void UHktVisibleStashComponent::SetProperty(FHktEntityId Entity, uint16 PropertyId, int32 Value)
{
    if (Entity >= MaxEntities || PropertyId >= MaxProperties)
        return;
    
    // 엔티티가 없으면 자동 생성 (스냅샷 적용 시)
    if (!ValidEntities[Entity])
    {
        ValidEntities[Entity] = true;
        if (Entity >= NextEntityId)
            NextEntityId = Entity + 1;
        
        // 속성 초기화
        for (int32 PropId = 0; PropId < MaxProperties; ++PropId)
        {
            Properties[PropId][Entity] = 0;
        }
    }
    
    Properties[PropertyId][Entity] = Value;
}

void UHktVisibleStashComponent::ApplyWrites(const TArray<FPendingWrite>& Writes)
{
    for (const auto& W : Writes)
    {
        SetProperty(W.Entity, W.PropertyId, W.Value);
    }
}

// ============================================================================
// Frame Management
// ============================================================================

void UHktVisibleStashComponent::MarkFrameCompleted(int32 FrameNumber)
{
    CompletedFrameNumber = FrameNumber;
}

// ============================================================================
// Snapshot Sync
// ============================================================================

void UHktVisibleStashComponent::ApplyEntitySnapshot(const FHktEntitySnapshot& Snapshot)
{
    if (!Snapshot.IsValid())
        return;
    
    FHktEntityId E = Snapshot.GetEntityId();
    
    if (E >= MaxEntities)
        return;
    
    // 엔티티 활성화
    ValidEntities[E] = true;
    if (E >= NextEntityId)
        NextEntityId = E + 1;
    
    // 속성 복사
    for (int32 PropId = 0; PropId < FMath::Min(Snapshot.Properties.Num(), MaxProperties); ++PropId)
    {
        Properties[PropId][E] = Snapshot.Properties[PropId];
    }
    
    UE_LOG(LogTemp, Verbose, TEXT("[VisibleStash] Applied snapshot for Entity %u"), E);
}

void UHktVisibleStashComponent::ApplySnapshots(const TArray<FHktEntitySnapshot>& Snapshots)
{
    for (const FHktEntitySnapshot& Snap : Snapshots)
    {
        ApplyEntitySnapshot(Snap);
    }
    
    UE_LOG(LogTemp, Log, TEXT("[VisibleStash] Applied %d snapshots"), Snapshots.Num());
}

void UHktVisibleStashComponent::Clear()
{
    ValidEntities.Init(false, MaxEntities);
    FreeList.Reset();
    NextEntityId = 0;
    CompletedFrameNumber = 0;
    
    for (int32 PropId = 0; PropId < MaxProperties; ++PropId)
    {
        FMemory::Memzero(Properties[PropId].GetData(), MaxEntities * sizeof(int32));
    }
}

// ============================================================================
// Checksum
// ============================================================================

uint32 UHktVisibleStashComponent::CalculateChecksum() const
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