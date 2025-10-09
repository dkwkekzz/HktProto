#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "HktClientCoreSubsystem.h"
#include "HktClientTestsUtil.h"
#include "HktFlagments.h"

// 간단한 서버-클라 연결 테스트
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHktClientCoreTest, "HktClient.Core", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FHktClientCoreTest::RunTest(const FString& Parameters)
{
    UWorld* World = HktClientTestsUtil::GetTestWorld();
    if (!TestNotNull(TEXT("월드를 찾을 수 없습니다."), World))
    {
        return false;
    }

    UHktClientCoreSubsystem* Subsystem = UHktClientCoreSubsystem::Get(World);
    if (!TestNotNull(TEXT("서브시스템을 가져올 수 없습니다."), Subsystem))
    {
        return false;
    }

    TestFalse(TEXT("서브시스템이 처음에는 연결되어 있지 않아야 합니다."), Subsystem->IsTickable());

    // 연결
    Subsystem->Connect();

    // 델리게이트 테스트
    int32 BehaviorCreatedCount = 0;
    FDelegateHandle CreatedHandle = Subsystem->OnBehaviorCreated.AddLambda(
        [&](const IHktBehavior& Behavior)
        {
            BehaviorCreatedCount++;
        });

    int32 BehaviorDestroyedCount = 0;
    FDelegateHandle DestroyedHandle = Subsystem->OnBehaviorDestroyed.AddLambda(
        [&](const IHktBehavior& Behavior)
        {
            BehaviorDestroyedCount++;
        });

	FMoveFlagment MoveFlagment;
	MoveFlagment.TargetLocation = FVector(100.0f, 200.0f, 300.0f);
	Subsystem->CreateBehavior(MoveFlagment);

    FJumpFlagment JumpFlagment;
    JumpFlagment.JumpHeight = 50.f;
    Subsystem->CreateBehavior(JumpFlagment);

    // 서버로부터 메시지를 받을 때까지 잠시 대기
    const float MessageWaitTime = 5.0f;
    FPlatformProcess::Sleep(MessageWaitTime);

    TestTrue(TEXT("OnBehaviorCreated 델리게이트가 한번 이상 호출되어야 합니다."), BehaviorCreatedCount > 0);
    TestTrue(TEXT("OnBehaviorDestroyed 델리게이트가 한번 이상 호출되어야 합니다."), BehaviorDestroyedCount > 0);

    Subsystem->OnBehaviorCreated.Remove(CreatedHandle);
    Subsystem->OnBehaviorDestroyed.Remove(DestroyedHandle);

    // 연결 끊기
    Subsystem->Disconnect();

    TestFalse(TEXT("서브시스템의 연결이 끊어져야 합니다."), Subsystem->IsTickable());

    return true;
}
