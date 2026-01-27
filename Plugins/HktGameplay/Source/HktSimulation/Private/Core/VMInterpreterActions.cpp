#include "VMInterpreter.h"
#include "VMProgram.h"
#include "VMStore.h"

// Helper
const FString& FVMInterpreter::GetString(FVMRuntime& Runtime, int32 Index)
{
    static FString Empty;
    if (Runtime.Program && Index >= 0 && Index < Runtime.Program->Strings.Num())
        return Runtime.Program->Strings[Index];
    return Empty;
}

// Entity Management
void FVMInterpreter::Op_SpawnEntity(FVMRuntime& Runtime, int32 StringIndex)
{
    const FString& ClassPath = GetString(Runtime, StringIndex);
    UE_LOG(LogTemp, Log, TEXT("[VM] SpawnEntity: %s"), *ClassPath);
    
    if (Stash)
    {
        EntityId NewEntity = Stash->AllocateEntity();
        Runtime.SetRegEntity(Reg::Spawned, NewEntity);
        
        // 소유자 설정 (Store를 통해 버퍼링)
        if (Runtime.Store)
        {
            Runtime.Store->WriteEntity(NewEntity, PropertyId::OwnerEntity, Runtime.GetRegEntity(Reg::Self));
            Runtime.Store->WriteEntity(NewEntity, PropertyId::EntityType, EntityType::Projectile);
        }
    }
}

void FVMInterpreter::Op_DestroyEntity(FVMRuntime& Runtime, RegisterIndex Entity)
{
    EntityId E = Runtime.GetRegEntity(Entity);
    UE_LOG(LogTemp, Log, TEXT("[VM] DestroyEntity: %u"), E);
    
    // 엔티티 제거는 즉시 적용 (다른 VM이 참조하지 못하게)
    if (Stash)
    {
        Stash->FreeEntity(E);
    }
}

// Position & Movement
void FVMInterpreter::Op_GetPosition(FVMRuntime& Runtime, RegisterIndex DstBase, RegisterIndex Entity)
{
    if (Runtime.Store)
    {
        EntityId E = Runtime.GetRegEntity(Entity);
        Runtime.SetReg(DstBase, Runtime.Store->ReadEntity(E, PropertyId::PosX));
        Runtime.SetReg(DstBase + 1, Runtime.Store->ReadEntity(E, PropertyId::PosY));
        Runtime.SetReg(DstBase + 2, Runtime.Store->ReadEntity(E, PropertyId::PosZ));
    }
}

void FVMInterpreter::Op_SetPosition(FVMRuntime& Runtime, RegisterIndex Entity, RegisterIndex SrcBase)
{
    if (Runtime.Store)
    {
        EntityId E = Runtime.GetRegEntity(Entity);
        Runtime.Store->WriteEntity(E, PropertyId::PosX, Runtime.GetReg(SrcBase));
        Runtime.Store->WriteEntity(E, PropertyId::PosY, Runtime.GetReg(SrcBase + 1));
        Runtime.Store->WriteEntity(E, PropertyId::PosZ, Runtime.GetReg(SrcBase + 2));
    }
}

void FVMInterpreter::Op_GetDistance(FVMRuntime& Runtime, RegisterIndex Dst, RegisterIndex Entity1, RegisterIndex Entity2)
{
    if (Runtime.Store)
    {
        EntityId E1 = Runtime.GetRegEntity(Entity1);
        EntityId E2 = Runtime.GetRegEntity(Entity2);
        
        int32 X1 = Runtime.Store->ReadEntity(E1, PropertyId::PosX);
        int32 Y1 = Runtime.Store->ReadEntity(E1, PropertyId::PosY);
        int32 Z1 = Runtime.Store->ReadEntity(E1, PropertyId::PosZ);
        
        int32 X2 = Runtime.Store->ReadEntity(E2, PropertyId::PosX);
        int32 Y2 = Runtime.Store->ReadEntity(E2, PropertyId::PosY);
        int32 Z2 = Runtime.Store->ReadEntity(E2, PropertyId::PosZ);
        
        int64 DX = X2 - X1;
        int64 DY = Y2 - Y1;
        int64 DZ = Z2 - Z1;
        
        int32 DistSq = static_cast<int32>(FMath::Min((int64)INT32_MAX, DX*DX + DY*DY + DZ*DZ));
        Runtime.SetReg(Dst, FMath::Sqrt(static_cast<float>(DistSq)));
    }
}

void FVMInterpreter::Op_MoveToward(FVMRuntime& Runtime, RegisterIndex Entity, RegisterIndex TargetBase, int32 Speed)
{
    if (Runtime.Store)
    {
        EntityId E = Runtime.GetRegEntity(Entity);
        Runtime.Store->WriteEntity(E, PropertyId::MoveTargetX, Runtime.GetReg(TargetBase));
        Runtime.Store->WriteEntity(E, PropertyId::MoveTargetY, Runtime.GetReg(TargetBase + 1));
        Runtime.Store->WriteEntity(E, PropertyId::MoveTargetZ, Runtime.GetReg(TargetBase + 2));
        Runtime.Store->WriteEntity(E, PropertyId::MoveSpeed, Speed);
        Runtime.Store->WriteEntity(E, PropertyId::IsMoving, 1);
    }
    UE_LOG(LogTemp, Log, TEXT("[VM] MoveToward: Entity %u, Speed %d"), Runtime.GetRegEntity(Entity), Speed);
}

void FVMInterpreter::Op_MoveForward(FVMRuntime& Runtime, RegisterIndex Entity, int32 Speed)
{
    if (Runtime.Store)
    {
        EntityId E = Runtime.GetRegEntity(Entity);
        Runtime.Store->WriteEntity(E, PropertyId::MoveSpeed, Speed);
        Runtime.Store->WriteEntity(E, PropertyId::IsMoving, 1);
    }
    UE_LOG(LogTemp, Log, TEXT("[VM] MoveForward: Entity %u, Speed %d"), Runtime.GetRegEntity(Entity), Speed);
}

void FVMInterpreter::Op_StopMovement(FVMRuntime& Runtime, RegisterIndex Entity)
{
    if (Runtime.Store)
    {
        EntityId E = Runtime.GetRegEntity(Entity);
        Runtime.Store->WriteEntity(E, PropertyId::IsMoving, 0);
    }
    UE_LOG(LogTemp, Log, TEXT("[VM] StopMovement: Entity %u"), Runtime.GetRegEntity(Entity));
}

// Spatial Query
void FVMInterpreter::Op_FindInRadius(FVMRuntime& Runtime, RegisterIndex CenterEntity, int32 RadiusCm)
{
    Runtime.SpatialQuery.Reset();
    
    if (Stash && Runtime.Store)
    {
        EntityId Center = Runtime.GetRegEntity(CenterEntity);
        
        // 중심 위치는 Store에서 읽기 (현재 VM의 로컬 캐시 반영)
        int32 CX = Runtime.Store->ReadEntity(Center, PropertyId::PosX);
        int32 CY = Runtime.Store->ReadEntity(Center, PropertyId::PosY);
        int32 CZ = Runtime.Store->ReadEntity(Center, PropertyId::PosZ);
        int32 Team = Runtime.Store->ReadEntity(Center, PropertyId::Team);
        
        int64 RadiusSq = static_cast<int64>(RadiusCm) * RadiusCm;
        
        // 다른 엔티티는 Stash에서 직접 읽기 (커밋된 상태)
        for (EntityId E = 0; E < 1024; ++E)
        {
            if (!Stash->IsValidEntity(E) || E == Center)
                continue;
            
            if (Stash->GetProperty(E, PropertyId::Team) == Team)
                continue;
            
            int32 EX = Stash->GetProperty(E, PropertyId::PosX);
            int32 EY = Stash->GetProperty(E, PropertyId::PosY);
            int32 EZ = Stash->GetProperty(E, PropertyId::PosZ);
            
            int64 DX = EX - CX;
            int64 DY = EY - CY;
            int64 DZ = EZ - CZ;
            
            if (DX*DX + DY*DY + DZ*DZ <= RadiusSq)
                Runtime.SpatialQuery.Entities.Add(E);
        }
    }
    
    Runtime.SetReg(Reg::Count, Runtime.SpatialQuery.Entities.Num());
    UE_LOG(LogTemp, Log, TEXT("[VM] FindInRadius: Found %d entities"), Runtime.SpatialQuery.Entities.Num());
}

void FVMInterpreter::Op_NextFound(FVMRuntime& Runtime)
{
    if (Runtime.SpatialQuery.HasNext())
    {
        Runtime.SetRegEntity(Reg::Iter, Runtime.SpatialQuery.Next());
        Runtime.SetReg(Reg::Flag, 1);
    }
    else
    {
        Runtime.SetRegEntity(Reg::Iter, InvalidEntityId);
        Runtime.SetReg(Reg::Flag, 0);
    }
}

// Combat
void FVMInterpreter::Op_ApplyDamage(FVMRuntime& Runtime, RegisterIndex Target, RegisterIndex Amount)
{
    EntityId E = Runtime.GetRegEntity(Target);
    int32 Dmg = Runtime.GetReg(Amount);
    
    UE_LOG(LogTemp, Log, TEXT("[VM] ApplyDamage: Entity %u takes %d damage"), E, Dmg);
    
    if (Runtime.Store && Stash && Stash->IsValidEntity(E))
    {
        int32 Health = Runtime.Store->ReadEntity(E, PropertyId::Health);
        int32 Defense = Runtime.Store->ReadEntity(E, PropertyId::Defense);
        
        int32 ActualDmg = FMath::Max(1, Dmg - Defense);
        int32 NewHealth = FMath::Max(0, Health - ActualDmg);
        
        Runtime.Store->WriteEntity(E, PropertyId::Health, NewHealth);
    }
}

void FVMInterpreter::Op_ApplyEffect(FVMRuntime& Runtime, RegisterIndex Target, int32 StringIndex)
{
    EntityId E = Runtime.GetRegEntity(Target);
    const FString& Effect = GetString(Runtime, StringIndex);
    UE_LOG(LogTemp, Log, TEXT("[VM] ApplyEffect: Entity %u, Effect %s"), E, *Effect);
}

void FVMInterpreter::Op_RemoveEffect(FVMRuntime& Runtime, RegisterIndex Target, int32 StringIndex)
{
    EntityId E = Runtime.GetRegEntity(Target);
    const FString& Effect = GetString(Runtime, StringIndex);
    UE_LOG(LogTemp, Log, TEXT("[VM] RemoveEffect: Entity %u, Effect %s"), E, *Effect);
}

// Animation & VFX
void FVMInterpreter::Op_PlayAnim(FVMRuntime& Runtime, RegisterIndex Entity, int32 StringIndex)
{
    UE_LOG(LogTemp, Log, TEXT("[VM] PlayAnim: Entity %u, Anim %s"), 
        Runtime.GetRegEntity(Entity), *GetString(Runtime, StringIndex));
}

void FVMInterpreter::Op_PlayAnimMontage(FVMRuntime& Runtime, RegisterIndex Entity, int32 StringIndex)
{
    UE_LOG(LogTemp, Log, TEXT("[VM] PlayAnimMontage: Entity %u, Montage %s"), 
        Runtime.GetRegEntity(Entity), *GetString(Runtime, StringIndex));
}

void FVMInterpreter::Op_StopAnim(FVMRuntime& Runtime, RegisterIndex Entity)
{
    UE_LOG(LogTemp, Log, TEXT("[VM] StopAnim: Entity %u"), Runtime.GetRegEntity(Entity));
}

void FVMInterpreter::Op_PlayVFX(FVMRuntime& Runtime, RegisterIndex PosBase, int32 StringIndex)
{
    UE_LOG(LogTemp, Log, TEXT("[VM] PlayVFX: (%d,%d,%d), VFX %s"), 
        Runtime.GetReg(PosBase), Runtime.GetReg(PosBase+1), Runtime.GetReg(PosBase+2),
        *GetString(Runtime, StringIndex));
}

void FVMInterpreter::Op_PlayVFXAttached(FVMRuntime& Runtime, RegisterIndex Entity, int32 StringIndex)
{
    UE_LOG(LogTemp, Log, TEXT("[VM] PlayVFXAttached: Entity %u, VFX %s"), 
        Runtime.GetRegEntity(Entity), *GetString(Runtime, StringIndex));
}

// Audio
void FVMInterpreter::Op_PlaySound(FVMRuntime& Runtime, int32 StringIndex)
{
    UE_LOG(LogTemp, Log, TEXT("[VM] PlaySound: %s"), *GetString(Runtime, StringIndex));
}

void FVMInterpreter::Op_PlaySoundAtLocation(FVMRuntime& Runtime, RegisterIndex PosBase, int32 StringIndex)
{
    UE_LOG(LogTemp, Log, TEXT("[VM] PlaySoundAtLocation: (%d,%d,%d), Sound %s"), 
        Runtime.GetReg(PosBase), Runtime.GetReg(PosBase+1), Runtime.GetReg(PosBase+2),
        *GetString(Runtime, StringIndex));
}

// Equipment
void FVMInterpreter::Op_SpawnEquipment(FVMRuntime& Runtime, RegisterIndex Owner, int32 Slot, int32 StringIndex)
{
    EntityId OwnerEntity = Runtime.GetRegEntity(Owner);
    const FString& EquipClass = GetString(Runtime, StringIndex);
    
    UE_LOG(LogTemp, Log, TEXT("[VM] SpawnEquipment: Owner %u, Slot %d, Class %s"), OwnerEntity, Slot, *EquipClass);
    
    if (Stash && Runtime.Store)
    {
        EntityId NewEquip = Stash->AllocateEntity();
        Runtime.Store->WriteEntity(NewEquip, PropertyId::EntityType, EntityType::Equipment);
        Runtime.Store->WriteEntity(NewEquip, PropertyId::OwnerEntity, OwnerEntity);
        Runtime.SetRegEntity(Reg::Spawned, NewEquip);
    }
}

// Utility
void FVMInterpreter::Op_Log(FVMRuntime& Runtime, int32 StringIndex)
{
    UE_LOG(LogTemp, Log, TEXT("[VM Log] %s"), *GetString(Runtime, StringIndex));
}