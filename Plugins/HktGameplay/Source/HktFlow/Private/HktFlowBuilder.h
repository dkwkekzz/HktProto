// Copyright (c) 2026 Hkt Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InstancedStruct.h"
#include "HktFlowTypes.h" // 분리된 타입 헤더 포함
#include "HktFlowBuilder.generated.h"

// ============================================================================
// [Node Structure]
// ============================================================================

UENUM(BlueprintType)
enum class EHktFlowOpCode : uint8
{
    None,
    PlayAnimation,
    SpawnEntity,
    MoveForward,
    DestroyEntity,
    SetDamage,
    SpawnEffect,
    
    // Scopes
    Scope_Root,
    Scope_OnEvent,
    Scope_OnSpawn,
    Scope_OnCollision,
    Scope_OnElapsed,
    Scope_OnWait,
    Scope_ForEachTarget,
};

/**
 * [Updated Param Wrapper]
 * - ContextName: 노드 실행 결과가 바인딩될 이름 (Output Context)
 * - Payload: 입력 데이터 (Input Data - FInstancedStruct)
 */
struct FHktFlowParam
{
    FName ContextName;      // Output Binding (Optional)
    FInstancedStruct Payload; // Polymorphic Data (Input)

    FHktFlowParam() = default;

    template<typename T>
    FHktFlowParam(FName InContext, const T& InData)
        : ContextName(InContext)
    {
        Payload.InitializeAs<T>(InData);
    }
};

/**
 * [Optimized Node Structure]
 * - ParamIndex로 간접 참조하는 대신, Param을 직접 포함하여 캐시 지역성(Cache Locality)을 극대화합니다.
 * - 노드 순회 시 데이터 점프(Cache Miss)를 방지합니다.
 */
struct FHktFlowNode
{
    EHktFlowOpCode OpCode;
    int32 ScopeEndIndex; 
    FHktFlowParam Param; // Co-located Data

    FHktFlowNode(EHktFlowOpCode InOp, const FHktFlowParam& InParam)
        : OpCode(InOp), ScopeEndIndex(-1), Param(InParam)
    {}
};

struct FHktFlowScopeProxy
{
    struct FHktFlowBuilder& Builder;
    int32 NodeIndex;

    struct FHktFlowBuilder& operator[](const struct FHktFlowBuilder& InBuilderResult);
};

// ============================================================================
// [Builder Implementation]
// ============================================================================

struct FHktFlowBuilder
{
public:
    // 단일 배열 관리 (Nodes가 OpCode와 Data를 모두 포함)
    TArray<FHktFlowNode>& Nodes;

    FHktFlowBuilder(TArray<FHktFlowNode>& InNodes)
        : Nodes(InNodes)
    {}

    // ------------------------------------------------------------------------
    // Actions
    // ------------------------------------------------------------------------

    FHktFlowBuilder& PlayAnimation(FName Subject, FGameplayTag AnimTag)
    {
        FHktParam_Animation Data;
        Data.SubjectName = Subject;
        Data.AnimTag = AnimTag;
        return AddNode(EHktFlowOpCode::PlayAnimation, NAME_None, Data);
    }

    FHktFlowBuilder& SpawnEntity(FGameplayTag EntityTag)
    {
        FHktParam_Spawn Data;
        Data.EntityTag = EntityTag;
        return AddNode(EHktFlowOpCode::SpawnEntity, NAME_None, Data);
    }

    FHktFlowBuilder& MoveForward(FName Subject, float Speed)
    {
        FHktParam_Move Data;
        Data.SubjectName = Subject;
        Data.Speed = Speed;
        return AddNode(EHktFlowOpCode::MoveForward, NAME_None, Data);
    }

    FHktFlowBuilder& DestroyEntity(FName Subject)
    {
        FHktParam_Move Data; // 이름만 필요하므로 재사용 예시
        Data.SubjectName = Subject;
        return AddNode(EHktFlowOpCode::DestroyEntity, NAME_None, Data);
    }

    FHktFlowBuilder& SetDamage(FName Target, float Power)
    {
        FHktParam_Damage Data;
        Data.TargetName = Target;
        Data.Power = Power;
        return AddNode(EHktFlowOpCode::SetDamage, NAME_None, Data);
    }

    FHktFlowBuilder& SpawnEffect(FGameplayTag EffectTag)
    {
        FHktParam_Spawn Data; // Spawn 구조체 재사용
        Data.EntityTag = EffectTag;
        return AddNode(EHktFlowOpCode::SpawnEffect, NAME_None, Data);
    }

    // ------------------------------------------------------------------------
    // Scopes
    // ------------------------------------------------------------------------

    FHktFlowScopeProxy OnSpawn(FName OutSpawnedContext)
    {
        return AddScopeNode(EHktFlowOpCode::Scope_OnSpawn, OutSpawnedContext, FHktParam_Base());
    }

    FHktFlowScopeProxy OnCollision(FName Subject, float Range, FName OutHitContext)
    {
        FHktParam_Collision Data;
        Data.SubjectName = Subject;
        Data.Radius = Range;
        return AddScopeNode(EHktFlowOpCode::Scope_OnCollision, OutHitContext, Data);
    }

    FHktFlowScopeProxy OnWait(FName Subject, FGameplayTag WaitSignalTag)
    {
        FHktParam_Wait Data;
        Data.SubjectName = Subject;
        Data.SignalTag = WaitSignalTag;
        return AddScopeNode(EHktFlowOpCode::Scope_OnWait, NAME_None, Data);
    }
    
    FHktFlowScopeProxy OnWait(float Duration)
    {
        FHktParam_Wait Data;
        Data.Duration = Duration;
        return AddScopeNode(EHktFlowOpCode::Scope_OnWait, NAME_None, Data);
    }

    FHktFlowScopeProxy OnElapsed(float Duration)
    {
        FHktParam_Wait Data;
        Data.Duration = Duration;
        return AddScopeNode(EHktFlowOpCode::Scope_OnElapsed, NAME_None, Data);
    }

    FHktFlowScopeProxy ForEachTarget(FName InTargetGroup, FName OutItContext)
    {
        FHktParam_Loop Data;
        Data.TargetGroupKey = InTargetGroup;
        return AddScopeNode(EHktFlowOpCode::Scope_ForEachTarget, OutItContext, Data);
    }

private:
    template<typename T>
    FHktFlowBuilder& AddNode(EHktFlowOpCode Op, FName OutContext, const T& InData)
    {
        // Param 구조체를 생성하여 Node에 바로 Emplace
        Nodes.Emplace(Op, FHktFlowParam(OutContext, InData));
        return *this;
    }

    template<typename T>
    FHktFlowScopeProxy AddScopeNode(EHktFlowOpCode Op, FName OutContext, const T& InData)
    {
        // Param 구조체를 생성하여 Node에 바로 Emplace하고 인덱스 반환
        int32 NodeIdx = Nodes.Emplace(Op, FHktFlowParam(OutContext, InData));
        return FHktFlowScopeProxy{ *this, NodeIdx };
    }
};

inline FHktFlowBuilder& FHktFlowScopeProxy::operator[](const FHktFlowBuilder& InBuilderResult)
{
    Builder.Nodes[NodeIndex].ScopeEndIndex = Builder.Nodes.Num();
    return Builder;
}