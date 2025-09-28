// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "HktClientViewSubsystem.h"

#ifdef HKTCLIENT_HktClientViewSubsystem_generated_h
#error "HktClientViewSubsystem.generated.h already included, missing '#pragma once' in HktClientViewSubsystem.h"
#endif
#define HKTCLIENT_HktClientViewSubsystem_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin Class UHktClientViewSubsystem **************************************************
HKTCLIENT_API UClass* Z_Construct_UClass_UHktClientViewSubsystem_NoRegister();

#define FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktClientViewSubsystem_h_10_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUHktClientViewSubsystem(); \
	friend struct Z_Construct_UClass_UHktClientViewSubsystem_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend HKTCLIENT_API UClass* Z_Construct_UClass_UHktClientViewSubsystem_NoRegister(); \
public: \
	DECLARE_CLASS2(UHktClientViewSubsystem, UWorldSubsystem, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/HktClient"), Z_Construct_UClass_UHktClientViewSubsystem_NoRegister) \
	DECLARE_SERIALIZER(UHktClientViewSubsystem)


#define FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktClientViewSubsystem_h_10_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UHktClientViewSubsystem(); \
	/** Deleted move- and copy-constructors, should never be used */ \
	UHktClientViewSubsystem(UHktClientViewSubsystem&&) = delete; \
	UHktClientViewSubsystem(const UHktClientViewSubsystem&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UHktClientViewSubsystem); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UHktClientViewSubsystem); \
	DEFINE_DEFAULT_CONSTRUCTOR_CALL(UHktClientViewSubsystem) \
	NO_API virtual ~UHktClientViewSubsystem();


#define FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktClientViewSubsystem_h_7_PROLOG
#define FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktClientViewSubsystem_h_10_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktClientViewSubsystem_h_10_INCLASS_NO_PURE_DECLS \
	FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktClientViewSubsystem_h_10_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class UHktClientViewSubsystem;

// ********** End Class UHktClientViewSubsystem ****************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktClientViewSubsystem_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
