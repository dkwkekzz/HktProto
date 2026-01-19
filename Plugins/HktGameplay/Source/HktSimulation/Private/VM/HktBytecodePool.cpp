// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktBytecodePool.h"

DEFINE_LOG_CATEGORY_STATIC(LogBytecodePool, Log, All);

TArray<TSharedPtr<FBytecodeBuffer>>& FBytecodePool::GetPool()
{
	static TArray<TSharedPtr<FBytecodeBuffer>> Pool;
	return Pool;
}

int32& FBytecodePool::GetMaxPoolSize()
{
	static int32 MaxPoolSize = 50;
	return MaxPoolSize;
}

TSharedPtr<FBytecodeBuffer> FBytecodePool::Acquire()
{
	TArray<TSharedPtr<FBytecodeBuffer>>& Pool = GetPool();

	if (Pool.Num() > 0)
	{
		// Reuse from pool
		TSharedPtr<FBytecodeBuffer> Buffer = Pool.Pop();
		Buffer->Reset(); // Clear contents but keep allocated memory

		UE_LOG(LogBytecodePool, Verbose, TEXT("Reused bytecode buffer from pool (Remaining: %d)"), Pool.Num());
		return Buffer;
	}
	else
	{
		// Create new buffer
		TSharedPtr<FBytecodeBuffer> NewBuffer = MakeShared<FBytecodeBuffer>();

		UE_LOG(LogBytecodePool, Verbose, TEXT("Created new bytecode buffer"));
		return NewBuffer;
	}
}

void FBytecodePool::Release(TSharedPtr<FBytecodeBuffer> Buffer)
{
	if (!Buffer.IsValid())
	{
		return;
	}

	TArray<TSharedPtr<FBytecodeBuffer>>& Pool = GetPool();

	// Check if we're at max pool size
	if (Pool.Num() >= GetMaxPoolSize())
	{
		// Let the buffer be destroyed
		UE_LOG(LogBytecodePool, Verbose, TEXT("Destroyed bytecode buffer (pool at max size: %d)"), GetMaxPoolSize());
		return;
	}

	// Reset and return to pool
	Buffer->Reset();
	Pool.Add(Buffer);

	UE_LOG(LogBytecodePool, Verbose, TEXT("Released bytecode buffer to pool (Available: %d)"), Pool.Num());
}

void FBytecodePool::Prewarm(int32 Count)
{
	TArray<TSharedPtr<FBytecodeBuffer>>& Pool = GetPool();

	for (int32 i = 0; i < Count; ++i)
	{
		TSharedPtr<FBytecodeBuffer> NewBuffer = MakeShared<FBytecodeBuffer>();
		Pool.Add(NewBuffer);
	}

	UE_LOG(LogBytecodePool, Log, TEXT("Prewarmed bytecode pool with %d buffers"), Count);
}

void FBytecodePool::Clear()
{
	TArray<TSharedPtr<FBytecodeBuffer>>& Pool = GetPool();
	Pool.Empty();

	UE_LOG(LogBytecodePool, Log, TEXT("Cleared bytecode pool"));
}

void FBytecodePool::GetStats(int32& OutTotalBuffers, int32& OutAvailableBuffers)
{
	TArray<TSharedPtr<FBytecodeBuffer>>& Pool = GetPool();
	OutTotalBuffers = Pool.Num();
	OutAvailableBuffers = Pool.Num();
}

void FBytecodePool::SetMaxPoolSize(int32 MaxSize)
{
	GetMaxPoolSize() = MaxSize;
	UE_LOG(LogBytecodePool, Log, TEXT("Set bytecode pool max size to %d"), MaxSize);
}
