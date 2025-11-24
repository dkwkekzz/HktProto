// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "HktMassPhysicsFragments.generated.h"

// 이동 목표 위치 Fragment
USTRUCT()
struct FHktMassVelocityFragment : public FMassFragment
{
	GENERATED_BODY()

	// 이동 속도
	UPROPERTY()
	FVector Value = FVector::ZeroVector;
};

// 누적된 힘 Fragment (매 프레임 초기화)
USTRUCT()
struct FHktMassForceFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY()
    FVector Value = FVector::ZeroVector;
};

// 물리 속성 Fragment (Rigidbody 유사)
USTRUCT()
struct FHktMassPhysicsParameters : public FMassConstSharedFragment
{
    GENERATED_BODY()

    // 질량 (Mass): 0이면 무한대 (움직이지 않음)
    UPROPERTY(EditAnywhere, Category = "Physics")
    float Mass = 1.0f;

    // 감쇠 계수 (Drag): 공기 저항/마찰 등 속도를 줄이는 힘의 비율
    UPROPERTY(EditAnywhere, Category = "Physics")
    float DragCoefficient = 5.0f;

    // 최대 속도 (MaxSpeed)
    UPROPERTY(EditAnywhere, Category = "Physics")
    float MaxSpeed = 600.0f;

    // 최대 힘 (MaxForce): 한 프레임에 가해질 수 있는 최대 힘 (급격한 가속 방지)
    UPROPERTY(EditAnywhere, Category = "Physics")
    float MaxForce = 2000.0f;
};

// 디버그 시각화용 Tag
USTRUCT()
struct FHktMassPhysicsDebugVisualizationTag : public FMassTag
{
    GENERATED_BODY()
};