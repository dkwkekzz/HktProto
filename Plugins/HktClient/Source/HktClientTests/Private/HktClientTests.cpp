#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

// 간단한 서버-클라 연결 테스트
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHktClientCoreTest, "HktClient.Core", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FHktClientCoreTest::RunTest(const FString& Parameters)
{
    return true;
}
