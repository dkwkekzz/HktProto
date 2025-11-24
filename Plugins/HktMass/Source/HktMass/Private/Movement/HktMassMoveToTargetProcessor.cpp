// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassMoveToTargetProcessor.h"
#include "HktMassMovementFragments.h"
#include "HktMassPhysicsFragments.h" // Physics Fragment
#include "HktMassDefines.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "Engine/World.h"
#include "MassExecutionContext.h"
#include "MassSimulationLOD.h"

UHktMassMoveToTargetProcessor::UHktMassMoveToTargetProcessor()
    : EntityQuery(*this)
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionOrder.ExecuteInGroup = HktMass::ExecuteGroupNames::Movement;
    ExecutionOrder.ExecuteAfter.Add(HktMass::ExecuteGroupNames::AI);
    ExecutionOrder.ExecuteAfter.Add(HktMass::ExecuteGroupNames::Squad);
}

void UHktMassMoveToTargetProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    EntityQuery.AddRequirement<FHktMassMoveToLocationFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FHktMassVelocityFragment>(EMassFragmentAccess::ReadOnly); // 현재 속도 참조용
    EntityQuery.AddRequirement<FHktMassForceFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddConstSharedRequirement<FHktMassPhysicsParameters>();

    EntityQuery.AddRequirement<FMassSimulationVariableTickFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
    EntityQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
    EntityQuery.SetChunkFilter(&FMassSimulationVariableTickChunkFragment::ShouldTickChunkThisFrame);
}

void UHktMassMoveToTargetProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& ExecutionContext)
{
    // 기본 설정값 (Physics Fragment가 없을 경우 대비 혹은 기본 로직용)
    const double AcceptanceRadius = 10.0;
    const double AcceptanceRadiusSq = AcceptanceRadius * AcceptanceRadius;
   
    // 정지 상태로 간주할 아주 작은 속도 (Deadzone)
    const double StopSpeedSq = 5.0 * 5.0;

    EntityQuery.ForEachEntityChunk(ExecutionContext, ([this, AcceptanceRadiusSq, StopSpeedSq](FMassExecutionContext& Context)
    {
        const FHktMassPhysicsParameters& Physics = Context.GetConstSharedFragment<FHktMassPhysicsParameters>();

        const TConstArrayView<FHktMassMoveToLocationFragment> TargetsList = Context.GetFragmentView<FHktMassMoveToLocationFragment>();
        const TConstArrayView<FTransformFragment> TransformsList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FHktMassForceFragment> ForcesList = Context.GetMutableFragmentView<FHktMassForceFragment>();
        const TConstArrayView<FHktMassVelocityFragment> VelocitiesList = Context.GetFragmentView<FHktMassVelocityFragment>();
       
        const TConstArrayView<FMassSimulationVariableTickFragment> SimVariableTickList = Context.GetFragmentView<FMassSimulationVariableTickFragment>();
        const bool bHasVariableTick = (SimVariableTickList.Num() > 0);
        const float WorldDeltaTime = Context.GetDeltaTimeSeconds();

        for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
        {
            const FVector TargetLoc = TargetsList[EntityIt].TargetLocation;
            const FTransform& Transform = TransformsList[EntityIt].GetTransform();
            FVector& CurrentForce = ForcesList[EntityIt].Value;
            const FVector CurrentVelocity = VelocitiesList[EntityIt].Value;

            const FVector CurrentLoc = Transform.GetLocation();
            FVector ToTarget = TargetLoc - CurrentLoc;
            const double DistSq = ToTarget.SizeSquared();
            const double Dist = FMath::Sqrt(DistSq);

            const float DeltaTime = bHasVariableTick ? SimVariableTickList[EntityIt].DeltaTime : WorldDeltaTime;

            if (DistSq > AcceptanceRadiusSq)
            {
                // Steering Behavior: Seek / Arrive
                const FVector Direction = ToTarget / Dist; // Normalize
               
                // Desired Velocity 계산
                // Physics의 MaxSpeed 사용
                double DesiredSpeed = Physics.MaxSpeed;

                // Arrive (감속) 처리: 목표에 가까워지면 속도를 줄임
                // 감속 시작 거리 (SlowingDistance)를 물리적으로 계산하거나 고정값 사용
                // v^2 = 2 * a * s 공식에서 멈추는데 필요한 거리 유추 가능하지만 간단히 설정
                const double SlowingDistance = 100.0; // 1m 전부터 감속
                if (Dist < SlowingDistance)
                {
                    // 거리에 비례하여 속도 줄임 (선형 감속)
                    DesiredSpeed *= (Dist / SlowingDistance);
                }

                FVector DesiredVelocity = Direction * DesiredSpeed;

                // Steering Force = (Desired Velocity - Current Velocity) * Gain
                // 반응성을 높이기 위해 Gain 사용 (스프링 상수의 역할)
                // 너무 높으면 Oscillation 발생, 너무 낮으면 반응 느림
                // 질량에 따라 힘을 조정 (F=ma)하여 일관된 반응성 유도
                const float SteeringGain = 4.0f * Physics.Mass;
               
                FVector SteeringForce = (DesiredVelocity - CurrentVelocity) * SteeringGain;

                // 급격한 방향 전환 방지 (힘의 변화율 제한은 상태가 필요하므로 여기선 힘의 크기만 제한)
                // MaxForce로 제한하는 것은 UpdateVelocityProcessor에서 수행하지만,
                // 조향력 자체가 너무 크지 않게 여기서도 조절 가능
               
                CurrentForce += SteeringForce;
            }
            else
            {
                // 목표 도달 (Arrival)
                // 정확히 멈추기 위해 반대 방향 힘을 가함 (Braking)
               
                // 현재 속도가 거의 0이면 힘을 가하지 않음 (Deadzone) -> 떨림 방지 핵심
                if (CurrentVelocity.SizeSquared() > StopSpeedSq)
                {
                     // 강력한 브레이킹 (Drag보다 강하게)
                     // 한 프레임만에 멈추려는 힘: F = -m * V / dt
                     // 하지만 너무 강하면 튕기므로 적절히 큰 계수 사용
                     const float BrakingCoeff = 10.0f * Physics.Mass;
                     CurrentForce += -CurrentVelocity * BrakingCoeff;
                }
                else
                {
                    // 완전히 정지 상태로 간주, 힘을 0으로 유지 (UpdateVelocity에서 미세 속도 0 처리됨)
                }
            }
        }
    }));
}

