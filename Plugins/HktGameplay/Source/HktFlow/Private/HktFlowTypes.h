// Copyright (c) 2026 Hkt Games. All Rights Reserved.

#pragma once

#include "HktFlowInterfaces.h"
#include "HktFlowTypes.generated.h"

// ============================================================================
// [Hkt Flow Data Types]
// FlowBuilder와 Runner에서 공유하는 데이터 구조체 정의 파일입니다.
// 빌더의 구현부와 데이터 스키마를 분리하여 컴파일 의존성을 낮춥니다.
// ============================================================================

/** 모든 파라미터 구조체의 기반 */
USTRUCT(BlueprintType)
struct FHktParam_Base
{
    GENERATED_BODY()
};

/** 엔티티 생성 관련 데이터 */
USTRUCT(BlueprintType)
struct FHktParam_Spawn : public FHktParam_Base
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag EntityTag;
};

/** 이동 관련 데이터 */
USTRUCT(BlueprintType)
struct FHktParam_Move : public FHktParam_Base
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SubjectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Speed = 0.0f;
};

/** 애니메이션 관련 데이터 */
USTRUCT(BlueprintType)
struct FHktParam_Animation : public FHktParam_Base
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SubjectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag AnimTag;
};

/** 충돌 감지 설정 데이터 */
USTRUCT(BlueprintType)
struct FHktParam_Collision : public FHktParam_Base
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SubjectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius = 0.0f;
};

/** 데미지 처리 데이터 */
USTRUCT(BlueprintType)
struct FHktParam_Damage : public FHktParam_Base
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TargetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Power = 0.0f;
};

/** 대기(Wait) 및 시그널 데이터 */
USTRUCT(BlueprintType)
struct FHktParam_Wait : public FHktParam_Base
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SubjectName; // Signal 대기일 경우 대상

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag SignalTag; // Signal 대기일 경우 태그

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 0.0f; // 시간 대기일 경우
};

/** 반복문 제어 데이터 */
USTRUCT(BlueprintType)
struct FHktParam_Loop : public FHktParam_Base
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TargetGroupKey;
};