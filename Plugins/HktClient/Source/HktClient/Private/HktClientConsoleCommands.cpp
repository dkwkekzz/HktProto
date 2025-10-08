#include "HktClientCoreSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

namespace HktClientConsoleCommands
{
    static FAutoConsoleCommandWithWorldAndArgs SendMovePacketCmd(
        TEXT("hkt.SendMovePacket"),
        TEXT("Sends a move packet. Usage: hkt.SendMovePacket <SubjectId> <X> <Y> <Z> <Speed>"),
        FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
            [](const TArray<FString>& Args, UWorld* World)
            {
            }
        )
    );

    static FAutoConsoleCommandWithWorldAndArgs SendJumpPacketCmd(
        TEXT("hkt.SendJumpPacket"),
        TEXT("Sends a jump packet. Usage: hkt.SendJumpPacket <SubjectId> <JumpHeight>"),
        FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
            [](const TArray<FString>& Args, UWorld* World)
            {
            }
        )
    );

    static FAutoConsoleCommandWithWorldAndArgs SendAttackPacketCmd(
        TEXT("hkt.SendAttackPacket"),
        TEXT("Sends an attack packet. Usage: hkt.SendAttackPacket <SubjectId> <SkillId> <TargetActorId>"),
        FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
            [](const TArray<FString>& Args, UWorld* World)
            {
            }
        )
    );
}


