#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InstancedStruct.h"
#include "HktJobBuilder.generated.h"

// ============================================================================
// 데이터 타입 정의
// ============================================================================

USTRUCT(BlueprintType)
struct FHktUnitHandle
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 Value = -1;

    bool IsValid() const { return Value >= 0; }
    bool operator==(const FHktUnitHandle& Other) const { return Value == Other.Value; }
};

// 델리게이트 선언
DECLARE_DELEGATE(FHktJobCallback);
DECLARE_DELEGATE_OneParam(FHktSpawnCallback, FHktUnitHandle);
DECLARE_DELEGATE_OneParam(FHktCollisionCallback, const TArray<FHktUnitHandle>&);

// ============================================================================
// Job Parameters (InstancedStruct용)
// ============================================================================

USTRUCT(BlueprintType)
struct FHktPlayAnimParams
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FHktUnitHandle Subject;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName AnimTag;
};

USTRUCT(BlueprintType)
struct FHktSpawnEntityParams
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName EntityTag;
};

USTRUCT(BlueprintType)
struct FHktMoveParams
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FHktUnitHandle Subject;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Speed = 0.0f;
};

USTRUCT(BlueprintType)
struct FHktDestroyParams
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FHktUnitHandle Subject;
};

USTRUCT(BlueprintType)
struct FHktDamageParams
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FHktUnitHandle Target;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Power = 0.0f;
};

USTRUCT(BlueprintType)
struct FHktSpawnEffectParams
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName EffectTag;
};

USTRUCT(BlueprintType)
struct FHktWaitParams
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Time = 0.0f;
};

USTRUCT(BlueprintType)
struct FHktCollisionParams
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FHktUnitHandle Subject;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Range = 0.0f;
};

// ============================================================================
// Job Command
// ============================================================================

enum class EHktJobOpCode : uint8
{
    None,
    PlayAnimation,
    SpawnEntity,
    MoveForward,
    DestroyEntity,
    SetDamage,
    SpawnEffect,
    Wait,
    RegisterSpawnCallback,
    RegisterCollisionCallback
};

struct FHktJobCommand
{
    EHktJobOpCode OpCode = EHktJobOpCode::None;
    int32 JobID = -1;

    FInstancedStruct Parameters;

    FHktJobCallback VoidCallback;
    FHktSpawnCallback SpawnCallback;
    FHktCollisionCallback CollisionCallback;
};

// ============================================================================
// Job Builder (Fluent Interface)
// ============================================================================

class HKTGAME_API FHktJobBuilder
{
public:
    FHktJobBuilder();
    virtual ~FHktJobBuilder();

    // ------------------------------------------------------------------------
    // 컨텍스트 바인딩
    // ------------------------------------------------------------------------
    template <typename T>
    T& BindContext()
    {
        ContextData.InitializeAs<T>();
        return ContextData.GetMutable<T>();
    }

    template <typename T>
    T* GetContext() { return ContextData.GetMutablePtr<T>(); }
    
    FInstancedStruct& GetRawContext() { return ContextData; }

    // ------------------------------------------------------------------------
    // 명령 큐 관리
    // ------------------------------------------------------------------------
    TArray<FHktJobCommand> FlushCommands();
    bool HasCommands() const { return PendingCommands.Num() > 0; }

    // ------------------------------------------------------------------------
    // Public API (Chainable Methods)
    // ------------------------------------------------------------------------
    
    // Actions (New Job ID 생성)
    FHktJobBuilder& PlayAnimation(FHktUnitHandle Subject, FName AnimTag);
    FHktJobBuilder& SpawnEntity(FName EntityTag);
    FHktJobBuilder& MoveForward(FHktUnitHandle Subject, float Speed);
    FHktJobBuilder& DestroyEntity(FHktUnitHandle Subject);
    FHktJobBuilder& SetDamage(FHktUnitHandle Target, float Power);
    FHktJobBuilder& SpawnEffect(FName EffectTag);

    // Flow Control (Last Job ID 참조)
    FHktJobBuilder& OnWait(float Time, FHktJobCallback Callback);
    FHktJobBuilder& OnSpawn(FHktSpawnCallback Callback);
    FHktJobBuilder& OnCollision(FHktUnitHandle Subject, float Range, FHktCollisionCallback Callback);
    FHktJobBuilder& OnCollisionMulti(FHktUnitHandle Subject, float Range, FHktCollisionCallback Callback);

private:
    // 내부 헬퍼
    void EnqueueCommand(const FHktJobCommand& Cmd);
    int32 GenerateJobID();

private:
    FInstancedStruct ContextData;
    TArray<FHktJobCommand> PendingCommands;
    
    int32 NextJobID = 0;
    int32 LastCreatedJobID = -1; // 체이닝을 위한 상태 추적
};