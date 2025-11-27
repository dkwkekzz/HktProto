// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassApplyVelocityProcessor.h"
#include "HktMassMovementFragments.h"
#include "HktMassPhysicsFragments.h" // Physics Fragment 추가
#include "HktMassDefines.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassSimulationLOD.h"

UHktMassApplyVelocityProcessor::UHktMassApplyVelocityProcessor()
    : EntityQuery(*this)
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = HktMass::ExecuteGroupNames::Physics_ApplyVelocity;
    ExecutionOrder.ExecuteAfter.Add(HktMass::ExecuteGroupNames::Collision);
}

void UHktMassApplyVelocityProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    EntityQuery.AddRequirement<FHktMassForceFragment>(EMassFragmentAccess::ReadWrite); // 힘을 읽고 초기화
    EntityQuery.AddRequirement<FHktMassVelocityFragment>(EMassFragmentAccess::ReadWrite); // 속도 업데이트
    EntityQuery.AddConstSharedRequirement<FHktMassPhysicsParameters>();
   
    EntityQuery.AddRequirement<FMassSimulationVariableTickFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
    EntityQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
    EntityQuery.SetChunkFilter(&FMassSimulationVariableTickChunkFragment::ShouldTickChunkThisFrame);
}

void UHktMassApplyVelocityProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    const float WorldDeltaTime = Context.GetDeltaTimeSeconds();

    EntityQuery.ForEachEntityChunk(Context, [WorldDeltaTime](FMassExecutionContext& LoopContext)
    {
        const FHktMassPhysicsParameters& Physics = LoopContext.GetConstSharedFragment<FHktMassPhysicsParameters>();

        const int32 NumEntities = LoopContext.GetNumEntities();
        const auto& Forces = LoopContext.GetMutableFragmentView<FHktMassForceFragment>();
        const auto& Velocities = LoopContext.GetMutableFragmentView<FHktMassVelocityFragment>();
       
        const auto& SimVariableTickList = LoopContext.GetFragmentView<FMassSimulationVariableTickFragment>();
        const bool bHasVariableTick = (SimVariableTickList.Num() > 0);

        for (int32 i = 0; i < NumEntities; ++i)
        {
            const float DeltaTime = bHasVariableTick ? SimVariableTickList[i].DeltaTime : WorldDeltaTime;
            FVector& Force = Forces[i].Value; // F_goal + F_ext
            FVector& Velocity = Velocities[i].Value; // v_old
           
            const float Mass = Physics.Mass;
           
            if (DeltaTime > KINDA_SMALL_NUMBER && Mass > KINDA_SMALL_NUMBER)
            {
                // 0. 외력(Steering + Repulsion) 제한
                // 한 프레임에 가해지는 힘이 MaxForce를 넘지 않도록 제한하여 튀는 현상 방지
                if (Force.SizeSquared() > Physics.MaxForce * Physics.MaxForce)
                {
                    Force = Force.GetSafeNormal() * Physics.MaxForce;
                }

                // 1. 마찰력(Friction/Drag) 계산 (Explicit Force)
                // F_friction = -v * k * m
                // 이전 속도(Velocity)를 사용하여 마찰력을 힘으로 변환
                FVector FrictionForce = -Velocity * Physics.DragCoefficient * Mass;

                // 2. 전체 힘 합산 (F_total = F_goal + F_ext + F_friction)
                FVector TotalForce = Force + FrictionForce;

                // 3. 가속도 계산 (a = F / m)
                FVector Acceleration = TotalForce / Mass;

                // 4. 속도 적분 (v_new = v_old + a * dt)
                Velocity += Acceleration * DeltaTime;

                // 5. 최대 속도 제한 (Safety)
                if (Velocity.SizeSquared() > Physics.MaxSpeed * Physics.MaxSpeed)
                {
                    Velocity = Velocity.GetSafeNormal() * Physics.MaxSpeed;
                }

                // 6. Sleep Check (미세 떨림 방지)
                if (Velocity.SizeSquared() < 10.0f)
                {
                    Velocity = FVector::ZeroVector;
                }
            }
           
            // 다음 프레임을 위해 누적된 힘 초기화
            Force = FVector::ZeroVector;
        }
    });
}


