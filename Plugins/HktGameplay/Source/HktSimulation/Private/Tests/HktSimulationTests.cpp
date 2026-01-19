// Copyright Hkt Studios, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "HktEntityManager.h"
#include "Core/HktSpatialIndex.h"
#include "Flow/FlowDefinitionRegistry.h"
#include "HktFlowOpcodes.h"
#include "VM/HktVMPool.h"
#include "VM/HktBytecodePool.h"

/**
 * Unit tests for HktSimulation module
 * Run with: Automation RunTests HktSimulation
 */

// Entity Manager Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHktEntityManagerTest, "HktSimulation.EntityManager.BasicOperations", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHktEntityManagerTest::RunTest(const FString& Parameters)
{
	FHktEntityManager Mgr;

	// Test player allocation
	FPlayerHandle Player = Mgr.AllocPlayer();
	TestTrue(TEXT("Player handle is valid"), Player.IsValid());

	// Test unit allocation
	FUnitHandle Unit1 = Mgr.AllocUnit(Player, FVector(0, 0, 0), FRotator::ZeroRotator);
	TestTrue(TEXT("Unit1 handle is valid"), Unit1.IsValid());
	TestTrue(TEXT("Unit1 is valid in manager"), Mgr.IsUnitValid(Unit1));

	// Test unit attributes
	FHktAttributeSet* Attrs = Mgr.GetUnitAttrs(Unit1);
	TestNotNull(TEXT("Unit1 attributes are accessible"), Attrs);
	TestEqual(TEXT("Unit1 default health"), Attrs->Get(EHktAttribute::Health), 100.0f);

	// Test unit freeing
	Mgr.FreeUnit(Unit1);
	TestFalse(TEXT("Unit1 is invalid after freeing"), Mgr.IsUnitValid(Unit1));

	// Test unit reuse
	FUnitHandle Unit2 = Mgr.AllocUnit(Player, FVector(100, 0, 0), FRotator::ZeroRotator);
	TestTrue(TEXT("Unit2 reuses freed index"), Unit2.Index == Unit1.Index);
	TestTrue(TEXT("Unit2 has different generation"), Unit2.Generation != Unit1.Generation);

	return true;
}

// Spatial Index Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHktSpatialIndexTest, "HktSimulation.SpatialIndex.QueryPerformance", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHktSpatialIndexTest::RunTest(const FString& Parameters)
{
	FHktSpatialIndex SpatialIndex(1000.0f); // 1000 unit cells

	// Insert test units
	TArray<FUnitHandle> TestUnits;
	const int32 NumUnits = 100;
	
	for (int32 i = 0; i < NumUnits; ++i)
	{
		FUnitHandle Handle(i, 1);
		FVector Location(i * 100.0f, i * 100.0f, 0.0f);
		SpatialIndex.Insert(Handle, Location);
		TestUnits.Add(Handle);
	}

	// Test sphere query
	TArray<FUnitHandle> Results;
	SpatialIndex.QuerySphere(FVector(5000, 5000, 0), 2000.0f, Results);
	
	TestTrue(TEXT("Sphere query returns results"), Results.Num() > 0);
	TestTrue(TEXT("Sphere query doesn't return all units"), Results.Num() < NumUnits);

	// Test removal
	SpatialIndex.Remove(TestUnits[0]);
	Results.Reset();
	SpatialIndex.QuerySphere(FVector(0, 0, 0), 500.0f, Results);
	TestFalse(TEXT("Removed unit not in query results"), Results.Contains(TestUnits[0]));

	return true;
}

// Flow Definition Registry Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHktFlowRegistryTest, "HktSimulation.FlowDefinitions.RegistryLookup", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHktFlowRegistryTest::RunTest(const FString& Parameters)
{
	// Note: Flow definitions are auto-registered, so we should find some
	int32 RegisteredCount = FFlowDefinitionRegistry::GetRegisteredCount();
	TestTrue(TEXT("Flow definitions are registered"), RegisteredCount > 0);

	TArray<FGameplayTag> Tags = FFlowDefinitionRegistry::GetRegisteredTags();
	TestEqual(TEXT("Tag count matches registry count"), Tags.Num(), RegisteredCount);

	return true;
}

// Opcode Registry Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHktOpcodeRegistryTest, "HktSimulation.Opcodes.RegistryOperations", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHktOpcodeRegistryTest::RunTest(const FString& Parameters)
{
	FHktOpRegistry::EnsureInitialized();

	// Test core opcodes are registered
	TestTrue(TEXT("WaitTime opcode is registered"), FHktOpRegistry::IsRegistered(ECoreOp::WaitTime));
	TestTrue(TEXT("ModifyAttribute opcode is registered"), FHktOpRegistry::IsRegistered(ECoreOp::ModifyAttribute));

	int32 RegisteredCount = FHktOpRegistry::GetRegisteredCount();
	TestTrue(TEXT("Multiple opcodes are registered"), RegisteredCount >= 6);

	// Test module range allocation
	HktOpCode StartOp = FHktOpRegistry::AllocateOpcodeRange(FName("TestModule"), 10);
	TestNotEqual(TEXT("Range allocation succeeds"), StartOp, static_cast<HktOpCode>(255));

	HktOpCode Start, End;
	TestTrue(TEXT("Module range can be queried"), FHktOpRegistry::GetModuleRange(FName("TestModule"), Start, End));
	TestEqual(TEXT("Range start matches allocated opcode"), Start, StartOp);
	TestEqual(TEXT("Range size is correct"), static_cast<int32>(End - Start), 10);

	return true;
}

// VM Pool Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHktVMPoolTest, "HktSimulation.VMPool.AcquireReleaseCycle", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHktVMPoolTest::RunTest(const FString& Parameters)
{
	FHktVMPool Pool;
	Pool.SetMaxPoolSize(10);
	Pool.Prewarm(5);

	int32 Total, Available, Active;
	Pool.GetStats(Total, Available, Active);
	
	TestEqual(TEXT("Prewarm creates correct number of VMs"), Total, 5);
	TestEqual(TEXT("All prewarmed VMs are available"), Available, 5);
	TestEqual(TEXT("No VMs are active initially"), Active, 0);

	// Acquire a VM
	FHktFlowVM* VM1 = Pool.Acquire(nullptr, nullptr, FUnitHandle());
	TestNotNull(TEXT("Acquired VM is valid"), VM1);

	Pool.GetStats(Total, Available, Active);
	TestEqual(TEXT("Available count decreases after acquire"), Available, 4);
	TestEqual(TEXT("Active count increases after acquire"), Active, 1);

	// Release the VM
	Pool.Release(VM1);
	Pool.GetStats(Total, Available, Active);
	TestEqual(TEXT("Available count increases after release"), Available, 5);
	TestEqual(TEXT("Active count decreases after release"), Active, 0);

	return true;
}

// Bytecode Pool Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHktBytecodePoolTest, "HktSimulation.BytecodePool.MemoryReuse", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHktBytecodePoolTest::RunTest(const FString& Parameters)
{
	FBytecodePool::Clear();
	FBytecodePool::Prewarm(3);

	int32 Total, Available;
	FBytecodePool::GetStats(Total, Available);
	TestEqual(TEXT("Prewarm creates buffers"), Available, 3);

	// Acquire a buffer
	TSharedPtr<FBytecodeBuffer> Buffer1 = FBytecodePool::Acquire();
	TestTrue(TEXT("Acquired buffer is valid"), Buffer1.IsValid());

	FBytecodePool::GetStats(Total, Available);
	TestEqual(TEXT("Available decreases after acquire"), Available, 2);

	// Release the buffer
	FBytecodePool::Release(Buffer1);
	FBytecodePool::GetStats(Total, Available);
	TestEqual(TEXT("Available increases after release"), Available, 3);

	FBytecodePool::Clear();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
