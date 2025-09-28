// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "HktClientCoreSubsystem.h"

#ifdef HKTCLIENT_HktClientCoreSubsystem_generated_h
#error "HktClientCoreSubsystem.generated.h already included, missing '#pragma once' in HktClientCoreSubsystem.h"
#endif
#define HKTCLIENT_HktClientCoreSubsystem_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin Class UHktClientCoreSubsystem **************************************************
HKTCLIENT_API UClass* Z_Construct_UClass_UHktClientCoreSubsystem_NoRegister();

#define FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktClientCoreSubsystem_h_13_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUHktClientCoreSubsystem(); \
	friend struct Z_Construct_UClass_UHktClientCoreSubsystem_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend HKTCLIENT_API UClass* Z_Construct_UClass_UHktClientCoreSubsystem_NoRegister(); \
public: \
	DECLARE_CLASS2(UHktClientCoreSubsystem, UGameInstanceSubsystem, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/HktClient"), Z_Construct_UClass_UHktClientCoreSubsystem_NoRegister) \
	DECLARE_SERIALIZER(UHktClientCoreSubsystem)


#define FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktClientCoreSubsystem_h_13_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UHktClientCoreSubsystem(); \
	/** Deleted move- and copy-constructors, should never be used */ \
	UHktClientCoreSubsystem(UHktClientCoreSubsystem&&) = delete; \
	UHktClientCoreSubsystem(const UHktClientCoreSubsystem&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UHktClientCoreSubsystem); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UHktClientCoreSubsystem); \
	DEFINE_DEFAULT_CONSTRUCTOR_CALL(UHktClientCoreSubsystem) \
	NO_API virtual ~UHktClientCoreSubsystem();


#define FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktClientCoreSubsystem_h_10_PROLOG
#define FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktClientCoreSubsystem_h_13_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktClientCoreSubsystem_h_13_INCLASS_NO_PURE_DECLS \
	FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktClientCoreSubsystem_h_13_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class UHktClientCoreSubsystem;

// ********** End Class UHktClientCoreSubsystem ****************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktClientCoreSubsystem_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
