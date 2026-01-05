#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "System/HktJobBuilder.h" // FHktJobCommand, FHktJobBuilder 참조
#include "HktJobProcessor.generated.h"

/**
 * JobBuilder에 의해 기록된 커맨드를 실제로 실행하는 처리기.
 * 타이머 관리, 액터 스폰, 충돌 감지 로직을 수행합니다.
 */
UCLASS()
class HKTGAME_API UHktJobProcessor : public UObject
{
    GENERATED_BODY()

public:
    /** 빌더에 쌓인 커맨드를 가져와 실행을 시작합니다. */
    void Process(FHktJobBuilder& InBuilder);

    /** 지속적인 업데이트가 필요할 경우 호출 (예: Tick에서 호출하거나 자체 Timer 사용) */
    void Tick(float DeltaTime);

protected:
    // 실제 실행 로직 구현부
    void ExecuteCommand(const FHktJobCommand& Cmd);
    
    // 개별 액션 구현
    void Exec_PlayAnimation(const FHktJobCommand& Cmd);
    void Exec_SpawnEntity(const FHktJobCommand& Cmd);
    void Exec_MoveForward(const FHktJobCommand& Cmd);
    void Exec_Wait(const FHktJobCommand& Cmd);
    
    // 콜백 처리
    void OnWaitFinished(FHktJobCallback Callback);
    void OnSpawnFinished(int32 JobID, FHktUnitHandle NewUnit);

private:
    /** 현재 처리 중인 빌더에 대한 참조 (추가 커맨드 수집용) */
    FHktJobBuilder* ActiveBuilder = nullptr;

    /** 비동기 처리를 위해 대기 중인 콜백들 (JobID -> Callback) */
    TMap<int32, FHktSpawnCallback> PendingSpawnCallbacks;
    TMap<int32, FHktCollisionCallback> PendingCollisionCallbacks;

    // 데모용: 간단한 Active Projectile 관리
    struct FActiveProjectile
    {
        FHktUnitHandle Handle;
        float Speed;
        float CollisionRange;
        int32 JobID_ForCollision; // 이 투사체에 연결된 충돌 Job ID
    };
    TArray<FActiveProjectile> ActiveProjectiles;
};