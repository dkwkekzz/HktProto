#include "System/HktJobBuilder.h"

// ============================================================================
// FHktJobBuilder Implementation
// ============================================================================

FHktJobBuilder::FHktJobBuilder() {}
FHktJobBuilder::~FHktJobBuilder() {}

TArray<FHktJobCommand> FHktJobBuilder::FlushCommands()
{
    TArray<FHktJobCommand> OutCommands = MoveTemp(PendingCommands);
    PendingCommands.Reset();
    return OutCommands;
}

void FHktJobBuilder::EnqueueCommand(const FHktJobCommand& Cmd)
{
    PendingCommands.Add(Cmd);
}

int32 FHktJobBuilder::GenerateJobID()
{
    LastCreatedJobID = ++NextJobID;
    return LastCreatedJobID;
}

// ----------------------------------------------------------------------------
// Actions (새로운 Job ID를 생성하고 상태를 갱신)
// ----------------------------------------------------------------------------

FHktJobBuilder& FHktJobBuilder::PlayAnimation(FHktUnitHandle Subject, FName AnimTag)
{
    int32 NewJobID = GenerateJobID();
    FHktJobCommand Cmd;
    Cmd.OpCode = EHktJobOpCode::PlayAnimation;
    Cmd.JobID = NewJobID;
    
    Cmd.Parameters.InitializeAs<FHktPlayAnimParams>();
    FHktPlayAnimParams& Params = Cmd.Parameters.GetMutable<FHktPlayAnimParams>();
    Params.Subject = Subject;
    Params.AnimTag = AnimTag;

    EnqueueCommand(Cmd);
    return *this;
}

FHktJobBuilder& FHktJobBuilder::SpawnEntity(FName EntityTag)
{
    int32 NewJobID = GenerateJobID();
    FHktJobCommand Cmd;
    Cmd.OpCode = EHktJobOpCode::SpawnEntity;
    Cmd.JobID = NewJobID;

    Cmd.Parameters.InitializeAs<FHktSpawnEntityParams>();
    Cmd.Parameters.GetMutable<FHktSpawnEntityParams>().EntityTag = EntityTag;

    EnqueueCommand(Cmd);
    return *this;
}

FHktJobBuilder& FHktJobBuilder::MoveForward(FHktUnitHandle Subject, float Speed)
{
    int32 NewJobID = GenerateJobID();
    FHktJobCommand Cmd;
    Cmd.OpCode = EHktJobOpCode::MoveForward;
    Cmd.JobID = NewJobID;

    Cmd.Parameters.InitializeAs<FHktMoveParams>();
    FHktMoveParams& Params = Cmd.Parameters.GetMutable<FHktMoveParams>();
    Params.Subject = Subject;
    Params.Speed = Speed;

    EnqueueCommand(Cmd);
    return *this;
}

FHktJobBuilder& FHktJobBuilder::DestroyEntity(FHktUnitHandle Subject)
{
    int32 NewJobID = GenerateJobID();
    FHktJobCommand Cmd;
    Cmd.OpCode = EHktJobOpCode::DestroyEntity;
    Cmd.JobID = NewJobID;

    Cmd.Parameters.InitializeAs<FHktDestroyParams>();
    Cmd.Parameters.GetMutable<FHktDestroyParams>().Subject = Subject;

    EnqueueCommand(Cmd);
    return *this;
}

FHktJobBuilder& FHktJobBuilder::SetDamage(FHktUnitHandle Target, float Power)
{
    int32 NewJobID = GenerateJobID();
    FHktJobCommand Cmd;
    Cmd.OpCode = EHktJobOpCode::SetDamage;
    Cmd.JobID = NewJobID;

    Cmd.Parameters.InitializeAs<FHktDamageParams>();
    FHktDamageParams& Params = Cmd.Parameters.GetMutable<FHktDamageParams>();
    Params.Target = Target;
    Params.Power = Power;

    EnqueueCommand(Cmd);
    return *this;
}

FHktJobBuilder& FHktJobBuilder::SpawnEffect(FName EffectTag)
{
    int32 NewJobID = GenerateJobID();
    FHktJobCommand Cmd;
    Cmd.OpCode = EHktJobOpCode::SpawnEffect;
    Cmd.JobID = NewJobID;

    Cmd.Parameters.InitializeAs<FHktSpawnEffectParams>();
    Cmd.Parameters.GetMutable<FHktSpawnEffectParams>().EffectTag = EffectTag;

    EnqueueCommand(Cmd);
    return *this;
}

// ----------------------------------------------------------------------------
// Flow Control (가장 최근 Job ID에 후속 작업 등록)
// ----------------------------------------------------------------------------

FHktJobBuilder& FHktJobBuilder::OnWait(float Time, FHktJobCallback Callback)
{
    // 이전 체이닝 단계에서 생성된 JobID를 사용
    int32 TargetJobID = LastCreatedJobID;
    
    FHktJobCommand Cmd;
    Cmd.OpCode = EHktJobOpCode::Wait;
    Cmd.JobID = TargetJobID;
    
    Cmd.Parameters.InitializeAs<FHktWaitParams>();
    Cmd.Parameters.GetMutable<FHktWaitParams>().Time = Time;
    
    Cmd.VoidCallback = Callback;
    EnqueueCommand(Cmd);
    
    return *this;
}

FHktJobBuilder& FHktJobBuilder::OnSpawn(FHktSpawnCallback Callback)
{
    int32 TargetJobID = LastCreatedJobID;

    FHktJobCommand Cmd;
    Cmd.OpCode = EHktJobOpCode::RegisterSpawnCallback;
    Cmd.JobID = TargetJobID;
    Cmd.SpawnCallback = Callback;
    EnqueueCommand(Cmd);
    
    return *this;
}

FHktJobBuilder& FHktJobBuilder::OnCollision(FHktUnitHandle Subject, float Range, FHktCollisionCallback Callback)
{
    int32 TargetJobID = LastCreatedJobID;

    FHktJobCommand Cmd;
    Cmd.OpCode = EHktJobOpCode::RegisterCollisionCallback;
    Cmd.JobID = TargetJobID;

    Cmd.Parameters.InitializeAs<FHktCollisionParams>();
    FHktCollisionParams& Params = Cmd.Parameters.GetMutable<FHktCollisionParams>();
    Params.Subject = Subject;
    Params.Range = Range;

    Cmd.CollisionCallback = Callback;
    EnqueueCommand(Cmd);
    
    return *this;
}

FHktJobBuilder& FHktJobBuilder::OnCollisionMulti(FHktUnitHandle Subject, float Range, FHktCollisionCallback Callback)
{
    // 로직이 동일하다면 재사용
    return OnCollision(Subject, Range, Callback);
}