// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Error codes for simulation operations
 */
enum class ESimulationError : uint8
{
	None = 0,
	InvalidHandle,
	InvalidEventData,
	FlowDefinitionNotFound,
	OpcodeNotFound,
	OutOfMemory,
	InvalidParameter,
	BuildFailed,
	ExecutionFailed,
	PoolExhausted,
	SpatialIndexFailed
};

/**
 * Result type for simulation operations
 * Provides error handling without exceptions
 */
struct HKTSIMULATION_API FSimulationResult
{
	bool bSuccess = true;
	ESimulationError Error = ESimulationError::None;
	FString ErrorMessage;
	FString Context; // Additional context (e.g., function name, event ID)

	/** Create a success result */
	static FSimulationResult Success()
	{
		return FSimulationResult{true, ESimulationError::None, TEXT(""), TEXT("")};
	}

	/** Create a failure result */
	static FSimulationResult Failure(ESimulationError InError, const FString& InMessage, const FString& InContext = TEXT(""))
	{
		return FSimulationResult{false, InError, InMessage, InContext};
	}

	/** Check if the operation was successful */
	bool IsSuccess() const { return bSuccess; }
	bool IsFailure() const { return !bSuccess; }

	/** Get error code name as string */
	FString GetErrorName() const
	{
		switch (Error)
		{
		case ESimulationError::None: return TEXT("None");
		case ESimulationError::InvalidHandle: return TEXT("InvalidHandle");
		case ESimulationError::InvalidEventData: return TEXT("InvalidEventData");
		case ESimulationError::FlowDefinitionNotFound: return TEXT("FlowDefinitionNotFound");
		case ESimulationError::OpcodeNotFound: return TEXT("OpcodeNotFound");
		case ESimulationError::OutOfMemory: return TEXT("OutOfMemory");
		case ESimulationError::InvalidParameter: return TEXT("InvalidParameter");
		case ESimulationError::BuildFailed: return TEXT("BuildFailed");
		case ESimulationError::ExecutionFailed: return TEXT("ExecutionFailed");
		case ESimulationError::PoolExhausted: return TEXT("PoolExhausted");
		case ESimulationError::SpatialIndexFailed: return TEXT("SpatialIndexFailed");
		default: return TEXT("Unknown");
		}
	}

	/** Get full error description */
	FString ToString() const
	{
		if (bSuccess)
		{
			return TEXT("Success");
		}

		FString Result = FString::Printf(TEXT("[%s] %s"), *GetErrorName(), *ErrorMessage);
		
		if (!Context.IsEmpty())
		{
			Result += FString::Printf(TEXT(" (Context: %s)"), *Context);
		}

		return Result;
	}

	/** Log this result */
	void LogResult(const FLogCategoryBase& Category, ELogVerbosity::Type Verbosity = ELogVerbosity::Warning) const
	{
		if (IsSuccess())
		{
			return; // Don't log successes by default
		}

		FMsg::Logf(__FILE__, __LINE__, Category.GetCategoryName(), Verbosity, TEXT("%s"), *ToString());
	}
};

/**
 * Result type that includes a value on success
 * Similar to std::expected or Rust's Result<T, E>
 */
template<typename T>
struct TSimulationResult
{
	bool bSuccess = false;
	ESimulationError Error = ESimulationError::None;
	FString ErrorMessage;
	FString Context;
	T Value;

	/** Create a success result with a value */
	static TSimulationResult Success(const T& InValue)
	{
		TSimulationResult Result;
		Result.bSuccess = true;
		Result.Error = ESimulationError::None;
		Result.Value = InValue;
		return Result;
	}

	/** Create a success result with a moved value */
	static TSimulationResult Success(T&& InValue)
	{
		TSimulationResult Result;
		Result.bSuccess = true;
		Result.Error = ESimulationError::None;
		Result.Value = MoveTemp(InValue);
		return Result;
	}

	/** Create a failure result */
	static TSimulationResult Failure(ESimulationError InError, const FString& InMessage, const FString& InContext = TEXT(""))
	{
		TSimulationResult Result;
		Result.bSuccess = false;
		Result.Error = InError;
		Result.ErrorMessage = InMessage;
		Result.Context = InContext;
		return Result;
	}

	/** Check if the operation was successful */
	bool IsSuccess() const { return bSuccess; }
	bool IsFailure() const { return !bSuccess; }

	/** Get the value (use only if IsSuccess()) */
	const T& GetValue() const
	{
		check(bSuccess); // Assert in debug builds
		return Value;
	}

	T& GetValue()
	{
		check(bSuccess);
		return Value;
	}

	/** Get the value or a default */
	T GetValueOr(const T& Default) const
	{
		return bSuccess ? Value : Default;
	}

	/** Convert to base FSimulationResult (losing the value) */
	FSimulationResult ToBaseResult() const
	{
		return FSimulationResult{bSuccess, Error, ErrorMessage, Context};
	}

	/** Get full error description */
	FString ToString() const
	{
		if (bSuccess)
		{
			return TEXT("Success");
		}

		FString Result = FString::Printf(TEXT("[%s] %s"), 
			*ToBaseResult().GetErrorName(), *ErrorMessage);
		
		if (!Context.IsEmpty())
		{
			Result += FString::Printf(TEXT(" (Context: %s)"), *Context);
		}

		return Result;
	}
};

/**
 * Helper macros for error handling
 */
#define SIM_RETURN_IF_FAILED(Result) \
	do { \
		if ((Result).IsFailure()) \
		{ \
			return (Result); \
		} \
	} while (0)

#define SIM_LOG_IF_FAILED(Result, Category) \
	do { \
		if ((Result).IsFailure()) \
		{ \
			(Result).LogResult(Category); \
		} \
	} while (0)
