// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "HktDataAsset.h"

#ifdef HKTCLIENT_HktDataAsset_generated_h
#error "HktDataAsset.generated.h already included, missing '#pragma once' in HktDataAsset.h"
#endif
#define HKTCLIENT_HktDataAsset_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin Class UHktDataAsset ************************************************************
HKTCLIENT_API UClass* Z_Construct_UClass_UHktDataAsset_NoRegister();

#define FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktDataAsset_h_10_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUHktDataAsset(); \
	friend struct Z_Construct_UClass_UHktDataAsset_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend HKTCLIENT_API UClass* Z_Construct_UClass_UHktDataAsset_NoRegister(); \
public: \
	DECLARE_CLASS2(UHktDataAsset, UPrimaryDataAsset, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/HktClient"), Z_Construct_UClass_UHktDataAsset_NoRegister) \
	DECLARE_SERIALIZER(UHktDataAsset)


#define FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktDataAsset_h_10_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UHktDataAsset(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	/** Deleted move- and copy-constructors, should never be used */ \
	UHktDataAsset(UHktDataAsset&&) = delete; \
	UHktDataAsset(const UHktDataAsset&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UHktDataAsset); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UHktDataAsset); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UHktDataAsset) \
	NO_API virtual ~UHktDataAsset();


#define FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktDataAsset_h_7_PROLOG
#define FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktDataAsset_h_10_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktDataAsset_h_10_INCLASS_NO_PURE_DECLS \
	FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktDataAsset_h_10_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class UHktDataAsset;

// ********** End Class UHktDataAsset **************************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktDataAsset_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
