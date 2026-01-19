// Copyright Hkt Studios, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "HktEntityManager.h"
#include "Core/HktSpatialIndex.h"
#include "VM/HktVMPool.h"
#include "HktFlowVM.h"

/**
 * Performance benchmarks for HktSimulation module
 * Run with: Automation RunTests HktSimulation.Benchmarks
 */

// Spatial Index Benchmark
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHktSpatialIndexBenchmark, "HktSimulation.Benchmarks.SpatialIndexVsLinear", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHktSpatialIndexBenchmark::RunTest(const FString& Parameters)
{
	const int32 NumEntities = 1000;
	const int32 NumQueries = 100;

	// Setup entity manager
	FHktEntityManager MgrWithIndex;
	FHktEntityManager MgrWithoutIndex;
	MgrWithoutIndex.SetSpatialIndexEnabled(false);

	// Create test entities
	TArray<FUnitHandle> Handles;
	for (int32 i = 0; i < NumEntities; ++i)
	{
		FVector Location(FMath::RandRange(-10000.0f, 10000.0f), 
						FMath::RandRange(-10000.0f, 10000.0f), 0.0f);
		
		FPlayerHandle Player;
		FUnitHandle Handle1 = MgrWithIndex.AllocUnit(Player, Location, FRotator::ZeroRotator);
		FUnitHandle Handle2 = MgrWithoutIndex.AllocUnit(Player, Location, FRotator::ZeroRotator);
		Handles.Add(Handle1);
	}

	// Benchmark with spatial index
	double TimeWithIndex = 0.0;
	{
		double StartTime = FPlatformTime::Seconds();
		for (int32 i = 0; i < NumQueries; ++i)
		{
			TArray<FUnitHandle> Results;
			FVector QueryCenter(FMath::RandRange(-10000.0f, 10000.0f), 
							   FMath::RandRange(-10000.0f, 10000.0f), 0.0f);
			MgrWithIndex.QueryUnitsInSphere(QueryCenter, 1000.0f, Results);
		}
		TimeWithIndex = FPlatformTime::Seconds() - StartTime;
	}

	// Benchmark without spatial index (linear search)
	double TimeWithoutIndex = 0.0;
	{
		double StartTime = FPlatformTime::Seconds();
		for (int32 i = 0; i < NumQueries; ++i)
		{
			TArray<FUnitHandle> Results;
			FVector QueryCenter(FMath::RandRange(-10000.0f, 10000.0f), 
							   FMath::RandRange(-10000.0f, 10000.0f), 0.0f);
			MgrWithoutIndex.QueryUnitsInSphere(QueryCenter, 1000.0f, Results);
		}
		TimeWithoutIndex = FPlatformTime::Seconds() - StartTime;
	}

	float Speedup = static_cast<float>(TimeWithoutIndex / TimeWithIndex);
	
	AddInfo(FString::Printf(TEXT("Spatial Index: %.4f seconds"), TimeWithIndex));
	AddInfo(FString::Printf(TEXT("Linear Search: %.4f seconds"), TimeWithoutIndex));
	AddInfo(FString::Printf(TEXT("Speedup: %.2fx faster with spatial index"), Speedup));

	TestTrue(TEXT("Spatial index is faster than linear search"), TimeWithIndex < TimeWithoutIndex);

	return true;
}

// VM Pool Benchmark
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHktVMPoolBenchmark, "HktSimulation.Benchmarks.VMPoolVsDirectAllocation", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHktVMPoolBenchmark::RunTest(const FString& Parameters)
{
	const int32 NumIterations = 1000;

	// Benchmark with pool
	double TimeWithPool = 0.0;
	{
		FHktVMPool Pool;
		Pool.Prewarm(10);
		
		double StartTime = FPlatformTime::Seconds();
		for (int32 i = 0; i < NumIterations; ++i)
		{
			FHktFlowVM* VM = Pool.Acquire(nullptr, nullptr, FUnitHandle());
			Pool.Release(VM);
		}
		TimeWithPool = FPlatformTime::Seconds() - StartTime;
	}

	// Benchmark without pool (direct allocation)
	double TimeWithoutPool = 0.0;
	{
		double StartTime = FPlatformTime::Seconds();
		for (int32 i = 0; i < NumIterations; ++i)
		{
			TUniquePtr<FHktFlowVM> VM = MakeUnique<FHktFlowVM>(nullptr, nullptr, FUnitHandle());
			// VM automatically destroyed
		}
		TimeWithoutPool = FPlatformTime::Seconds() - StartTime;
	}

	float Speedup = static_cast<float>(TimeWithoutPool / TimeWithPool);
	
	AddInfo(FString::Printf(TEXT("With Pool: %.4f seconds"), TimeWithPool));
	AddInfo(FString::Printf(TEXT("Without Pool: %.4f seconds"), TimeWithoutPool));
	AddInfo(FString::Printf(TEXT("Speedup: %.2fx faster with pooling"), Speedup));

	TestTrue(TEXT("VM pooling is faster than direct allocation"), TimeWithPool < TimeWithoutPool);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
