// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Wrapper for a bytecode buffer
 * Allows for pooling and reuse
 */
class HKTSIMULATION_API FBytecodeBuffer
{
public:
	/**
	 * Get the underlying byte array
	 */
	TArray<uint8>& GetBuffer() { return Buffer; }
	const TArray<uint8>& GetBuffer() const { return Buffer; }

	/**
	 * Reset the buffer (clears contents but keeps allocated memory)
	 */
	void Reset() { Buffer.Reset(); }

	/**
	 * Clear the buffer completely (releases memory)
	 */
	void Empty() { Buffer.Empty(); }

	/**
	 * Get the current size
	 */
	int32 Num() const { return Buffer.Num(); }

	/**
	 * Get the allocated capacity
	 */
	int32 GetAllocatedSize() const { return Buffer.GetAllocatedSize(); }

private:
	TArray<uint8> Buffer;
};

/**
 * Object pool for bytecode buffers
 * Reduces allocation overhead for temporary bytecode generation
 */
class HKTSIMULATION_API FBytecodePool
{
public:
	/**
	 * Acquire a bytecode buffer from the pool
	 * @return A bytecode buffer (either from pool or newly created)
	 */
	static TSharedPtr<FBytecodeBuffer> Acquire();

	/**
	 * Release a bytecode buffer back to the pool
	 * @param Buffer - The buffer to release (will be reset for reuse)
	 */
	static void Release(TSharedPtr<FBytecodeBuffer> Buffer);

	/**
	 * Prewarm the pool
	 * @param Count - Number of buffers to create
	 */
	static void Prewarm(int32 Count);

	/**
	 * Clear the pool
	 */
	static void Clear();

	/**
	 * Get pool statistics
	 */
	static void GetStats(int32& OutTotalBuffers, int32& OutAvailableBuffers);

	/**
	 * Set the maximum pool size (default: 50)
	 */
	static void SetMaxPoolSize(int32 MaxSize);

private:
	/**
	 * Get the singleton pool instance (Meyers Singleton)
	 */
	static TArray<TSharedPtr<FBytecodeBuffer>>& GetPool();
	static int32& GetMaxPoolSize();
};
