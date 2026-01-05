#include "System/HktJobProcessor.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogHktProcessor, Log, All);

void UHktJobProcessor::Process(FHktJobBuilder& InBuilder)
{
    ActiveBuilder = &InBuilder;
    
    // 빌더에 쌓인 커맨드를 모두 가져와서 즉시 실행
    TArray<FHktJobCommand> Commands = ActiveBuilder->FlushCommands();
    
    for (const FHktJobCommand& Cmd : Commands)
    {
        ExecuteCommand(Cmd);
    }
}

void UHktJobProcessor::Tick(float DeltaTime)
{
    // 이동 로직 및 충돌 체크 시뮬레이션
    for (int32 i = ActiveProjectiles.Num() - 1; i >= 0; --i)
    {
        FActiveProjectile& Proj = ActiveProjectiles[i];
        
        // 1. 이동 시뮬레이션 (가상)
        // UE_LOG(LogHktProcessor, Verbose, TEXT("Unit %d Moving... Speed: %f"), Proj.Handle.Value, Proj.Speed);

        // 2. 충돌 시뮬레이션 (가상: 매 틱마다 5% 확률로 충돌 발생)
        bool bHit = (FMath::RandRange(0.0f, 1.0f) < 0.05f); 
        
        if (bHit && PendingCollisionCallbacks.Contains(Proj.JobID_ForCollision))
        {
            UE_LOG(LogHktProcessor, Log, TEXT("Unit %d HIT something!"), Proj.Handle.Value);
            
            // 충돌 대상 수집 (가상 데이터)
            TArray<FHktUnitHandle> Targets;
            FHktUnitHandle HitTarget; HitTarget.Value = 777;
            Targets.Add(HitTarget);

            // 콜백 실행
            FHktCollisionCallback& Callback = PendingCollisionCallbacks[Proj.JobID_ForCollision];
            if (Callback.IsBound())
            {
                Callback.Execute(Targets);
                
                // 새로 쌓인 커맨드 처리
                if (ActiveBuilder)
                {
                    Process(*ActiveBuilder);
                }
            }
            
            // 시뮬레이션 목록에서 제거
            ActiveProjectiles.RemoveAt(i);
        }
    }
}

void UHktJobProcessor::ExecuteCommand(const FHktJobCommand& Cmd)
{
    // 각 함수 내부에서 InstancedStruct의 유효성을 검사하며 파라미터를 추출합니다.
    switch (Cmd.OpCode)
    {
    case EHktJobOpCode::PlayAnimation: Exec_PlayAnimation(Cmd); break;
    case EHktJobOpCode::SpawnEntity:   Exec_SpawnEntity(Cmd); break;
    case EHktJobOpCode::MoveForward:   Exec_MoveForward(Cmd); break;
    case EHktJobOpCode::Wait:          Exec_Wait(Cmd); break;
    
    case EHktJobOpCode::RegisterSpawnCallback:
        PendingSpawnCallbacks.Add(Cmd.JobID, Cmd.SpawnCallback);
        break;
        
    case EHktJobOpCode::RegisterCollisionCallback:
        PendingCollisionCallbacks.Add(Cmd.JobID, Cmd.CollisionCallback);
        break;

    case EHktJobOpCode::DestroyEntity:
        if (const FHktDestroyParams* Params = Cmd.Parameters.GetPtr<FHktDestroyParams>())
        {
            UE_LOG(LogHktProcessor, Log, TEXT("EXEC: Destroy Unit %d"), Params->Subject.Value);
        }
        break;

    case EHktJobOpCode::SetDamage:
        if (const FHktDamageParams* Params = Cmd.Parameters.GetPtr<FHktDamageParams>())
        {
            UE_LOG(LogHktProcessor, Log, TEXT("EXEC: Damage %f to Unit %d"), Params->Power, Params->Target.Value);
        }
        break;

    case EHktJobOpCode::SpawnEffect:
        if (const FHktSpawnEffectParams* Params = Cmd.Parameters.GetPtr<FHktSpawnEffectParams>())
        {
            UE_LOG(LogHktProcessor, Log, TEXT("EXEC: Spawn Effect %s"), *Params->EffectTag.ToString());
        }
        break;
        
    default: break;
    }
}

// ----------------------------------------------------------------------------
// Action Executors
// ----------------------------------------------------------------------------

void UHktJobProcessor::Exec_PlayAnimation(const FHktJobCommand& Cmd)
{
    if (const FHktPlayAnimParams* Params = Cmd.Parameters.GetPtr<FHktPlayAnimParams>())
    {
        UE_LOG(LogHktProcessor, Log, TEXT("EXEC: PlayAnimation %s on Unit %d"), *Params->AnimTag.ToString(), Params->Subject.Value);
    }
}

void UHktJobProcessor::Exec_SpawnEntity(const FHktJobCommand& Cmd)
{
    const FHktSpawnEntityParams* Params = Cmd.Parameters.GetPtr<FHktSpawnEntityParams>();
    if (!Params) return;

    UE_LOG(LogHktProcessor, Log, TEXT("EXEC: Spawning Entity %s... (Async)"), *Params->EntityTag.ToString());

    // 예시: 0.1초 뒤 스폰 완료 처리
    FTimerHandle Handle;
    FTimerDelegate TimerDel;
    TimerDel.BindUObject(this, &UHktJobProcessor::OnSpawnFinished, Cmd.JobID, FHktUnitHandle{100 + Cmd.JobID});
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(Handle, TimerDel, 0.1f, false);
    }
}

void UHktJobProcessor::Exec_MoveForward(const FHktJobCommand& Cmd)
{
    const FHktMoveParams* Params = Cmd.Parameters.GetPtr<FHktMoveParams>();
    if (!Params) return;

    UE_LOG(LogHktProcessor, Log, TEXT("EXEC: Unit %d Start Moving Forward Speed %f"), Params->Subject.Value, Params->Speed);
    
    // 시뮬레이션용 데이터 등록
    FActiveProjectile Proj;
    Proj.Handle = Params->Subject;
    Proj.Speed = Params->Speed;
    
    // 충돌 Job 매핑 (단순화: 현재 존재하는 모든 충돌 콜백을 매핑)
    for (auto& Pair : PendingCollisionCallbacks)
    {
        Proj.JobID_ForCollision = Pair.Key; 
    }
    
    ActiveProjectiles.Add(Proj);
}

void UHktJobProcessor::Exec_Wait(const FHktJobCommand& Cmd)
{
    const FHktWaitParams* Params = Cmd.Parameters.GetPtr<FHktWaitParams>();
    if (!Params) return;

    UE_LOG(LogHktProcessor, Log, TEXT("EXEC: Wait for %f seconds"), Params->Time);

    if (Cmd.VoidCallback.IsBound())
    {
        FTimerHandle Handle;
        FTimerDelegate TimerDel;
        TimerDel.BindUObject(this, &UHktJobProcessor::OnWaitFinished, Cmd.VoidCallback);
        
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().SetTimer(Handle, TimerDel, Params->Time, false);
        }
    }
}

// ----------------------------------------------------------------------------
// Internal Callbacks
// ----------------------------------------------------------------------------

void UHktJobProcessor::OnWaitFinished(FHktJobCallback Callback)
{
    UE_LOG(LogHktProcessor, Log, TEXT("Wait Finished. Executing Callback block..."));

    Callback.ExecuteIfBound();

    if (ActiveBuilder)
    {
        Process(*ActiveBuilder);
    }
}

void UHktJobProcessor::OnSpawnFinished(int32 JobID, FHktUnitHandle NewUnit)
{
    UE_LOG(LogHktProcessor, Log, TEXT("Spawn Finished. Unit ID: %d"), NewUnit.Value);

    if (PendingSpawnCallbacks.Contains(JobID))
    {
        FHktSpawnCallback& Callback = PendingSpawnCallbacks[JobID];
        
        Callback.ExecuteIfBound(NewUnit);
        
        if (ActiveBuilder)
        {
            Process(*ActiveBuilder);
        }
        
        PendingSpawnCallbacks.Remove(JobID);
    }
}