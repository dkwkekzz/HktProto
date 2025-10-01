#include "HktClientCoreSubsystem.h"
#include "HktPacketTypes.h"
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
                if (Args.Num() < 5)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Usage: hkt.SendMovePacket <SubjectId> <X> <Y> <Z> <Speed>"));
                    return;
                }

                if (UHktClientCoreSubsystem* Core = UHktClientCoreSubsystem::Get(World))
                {
                    const int64 SubjectId = FCString::Atoi64(*Args[0]);
                    FMovePacket Packet;
                    Packet.TargetLocation.X = FCString::Atof(*Args[1]);
                    Packet.TargetLocation.Y = FCString::Atof(*Args[2]);
                    Packet.TargetLocation.Z = FCString::Atof(*Args[3]);
                    Packet.Speed = FCString::Atof(*Args[4]);

                    Core->ExecuteBehavior(SubjectId, Packet);
                    UE_LOG(LogTemp, Log, TEXT("Sent Move Packet"));
                }
            }
        )
    );

    static FAutoConsoleCommandWithWorldAndArgs SendJumpPacketCmd(
        TEXT("hkt.SendJumpPacket"),
        TEXT("Sends a jump packet. Usage: hkt.SendJumpPacket <SubjectId> <JumpHeight>"),
        FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
            [](const TArray<FString>& Args, UWorld* World)
            {
                if (Args.Num() < 2)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Usage: hkt.SendJumpPacket <SubjectId> <JumpHeight>"));
                    return;
                }

                if (UHktClientCoreSubsystem* Core = UHktClientCoreSubsystem::Get(World))
                {
                    const int64 SubjectId = FCString::Atoi64(*Args[0]);
                    FJumpPacket Packet;
                    Packet.JumpHeight = FCString::Atof(*Args[1]);

                    Core->ExecuteBehavior(SubjectId, Packet);
                    UE_LOG(LogTemp, Log, TEXT("Sent Jump Packet"));
                }
            }
        )
    );

    static FAutoConsoleCommandWithWorldAndArgs SendAttackPacketCmd(
        TEXT("hkt.SendAttackPacket"),
        TEXT("Sends an attack packet. Usage: hkt.SendAttackPacket <SubjectId> <SkillId> <TargetActorId>"),
        FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
            [](const TArray<FString>& Args, UWorld* World)
            {
                if (Args.Num() < 3)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Usage: hkt.SendAttackPacket <SubjectId> <SkillId> <TargetActorId>"));
                    return;
                }

                if (UHktClientCoreSubsystem* Core = UHktClientCoreSubsystem::Get(World))
                {
                    const int64 SubjectId = FCString::Atoi64(*Args[0]);
                    FAttackPacket Packet;
                    Packet.SkillId = FCString::Atoi(*Args[1]);
                    Packet.TargetActorId = FCString::Atoi64(*Args[2]);

                    Core->ExecuteBehavior(SubjectId, Packet);
                    UE_LOG(LogTemp, Log, TEXT("Sent Attack Packet"));
                }
            }
        )
    );

    static FAutoConsoleCommandWithWorldAndArgs SyncGroupCmd(
        TEXT("hkt.SyncGroup"),
        TEXT("Syncs a group. Usage: hkt.SyncGroup <SubjectId> <GroupId>"),
        FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
            [](const TArray<FString>& Args, UWorld* World)
            {
                if (Args.Num() < 2)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Usage: hkt.SyncGroup <SubjectId> <GroupId>"));
                    return;
                }

                if (UHktClientCoreSubsystem* Core = UHktClientCoreSubsystem::Get(World))
                {
                    const int64 SubjectId = FCString::Atoi64(*Args[0]);
                    const int64 GroupId = FCString::Atoi64(*Args[1]);

                    Core->SyncGroup(SubjectId, GroupId);
                    UE_LOG(LogTemp, Log, TEXT("Sent SyncGroup for SubjectId: %lld, GroupId: %lld"), SubjectId, GroupId);
                }
            }
        )
    );
}


