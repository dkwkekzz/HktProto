#include "CoreMinimal.h"
#include "HktClientCoreSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HktFlagments.h"
#include "HktLocalServer.h"

namespace HktClientConsoleCommands
{
    FHktLocalServer GLocalServer;

    static FAutoConsoleCommandWithWorldAndArgs LocalServerCmd(
        TEXT("hkt.LocalServer"),
        TEXT("Starts a local server."),
        FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
            [](const TArray<FString>& Args, UWorld* World)
            {
                GLocalServer.Start();
            }
        )
    );
    
    static FAutoConsoleCommand StopLocalServerCmd(
        TEXT("hkt.StopLocalServer"),
        TEXT("Stops the local server."),
        FConsoleCommandDelegate::CreateLambda(
            []()
            {
                GLocalServer.Stop();
            }
        )
    );

    static FAutoConsoleCommandWithWorld ConnectCmd(
        TEXT("hkt.Connect"),
        TEXT("Connects to the server."),
        FConsoleCommandWithWorldDelegate::CreateLambda(
            [](UWorld* World)
            {
                if (auto* Subsystem = UHktClientCoreSubsystem::Get(World))
                {
                    Subsystem->Connect();
                }
            }
        )
    );

    static FAutoConsoleCommandWithWorld DisconnectCmd(
        TEXT("hkt.Disconnect"),
        TEXT("Disconnects from the server."),
        FConsoleCommandWithWorldDelegate::CreateLambda(
            [](UWorld* World)
            {
                if (auto* Subsystem = UHktClientCoreSubsystem::Get(World))
                {
                    Subsystem->Disconnect();
                }
            }
        )
    );

    static FAutoConsoleCommandWithWorldAndArgs SendMovePacketCmd(
        TEXT("hkt.SendMovePacket"),
        TEXT("Sends a move packet. Usage: hkt.SendMovePacket <X> <Y> <Z> <Speed>"),
        FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
            [](const TArray<FString>& Args, UWorld* World)
            {
                if (Args.Num() < 4)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Usage: hkt.SendMovePacket <X> <Y> <Z> <Speed>"));
                    return;
                }

                if (auto* Subsystem = UHktClientCoreSubsystem::Get(World))
                {
                    FMoveFlagment Flagment;
                    Flagment.TargetLocation.X = FCString::Atof(*Args[0]);
                    Flagment.TargetLocation.Y = FCString::Atof(*Args[1]);
                    Flagment.TargetLocation.Z = FCString::Atof(*Args[2]);
                    Flagment.Speed = FCString::Atof(*Args[3]);
                    Subsystem->CreateBehavior(Flagment);
                }
            }
        )
    );

    static FAutoConsoleCommandWithWorldAndArgs SendJumpPacketCmd(
        TEXT("hkt.SendJumpPacket"),
        TEXT("Sends a jump packet. Usage: hkt.SendJumpPacket <JumpHeight>"),
        FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
            [](const TArray<FString>& Args, UWorld* World)
            {
                if (Args.Num() < 1)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Usage: hkt.SendJumpPacket <JumpHeight>"));
                    return;
                }
                
                if (auto* Subsystem = UHktClientCoreSubsystem::Get(World))
                {
                    FJumpFlagment Flagment;
                    Flagment.JumpHeight = FCString::Atof(*Args[0]);
                    Subsystem->CreateBehavior(Flagment);
                }
            }
        )
    );

    static FAutoConsoleCommandWithWorldAndArgs SendAttackPacketCmd(
        TEXT("hkt.SendAttackPacket"),
        TEXT("Sends an attack packet. Usage: hkt.SendAttackPacket <SkillId> <TargetActorId>"),
        FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
            [](const TArray<FString>& Args, UWorld* World)
            {
                if (Args.Num() < 2)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Usage: hkt.SendAttackPacket <SkillId> <TargetActorId>"));
                    return;
                }

                if (auto* Subsystem = UHktClientCoreSubsystem::Get(World))
                {
                    FAttackFlagment Flagment;
                    Flagment.SkillId = FCString::Atoi(*Args[0]);
                    Flagment.TargetActorId = FCString::Atoi64(*Args[1]);
                    Subsystem->CreateBehavior(Flagment);
                }
            }
        )
    );
}


