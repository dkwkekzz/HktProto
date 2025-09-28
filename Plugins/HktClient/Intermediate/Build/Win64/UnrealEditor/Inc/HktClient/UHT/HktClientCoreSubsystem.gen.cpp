// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "HktClientCoreSubsystem.h"
#include "Engine/GameInstance.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void EmptyLinkFunctionForGeneratedCodeHktClientCoreSubsystem() {}

// ********** Begin Cross Module References ********************************************************
ENGINE_API UClass* Z_Construct_UClass_UGameInstanceSubsystem();
HKTCLIENT_API UClass* Z_Construct_UClass_UHktClientCoreSubsystem();
HKTCLIENT_API UClass* Z_Construct_UClass_UHktClientCoreSubsystem_NoRegister();
UPackage* Z_Construct_UPackage__Script_HktClient();
// ********** End Cross Module References **********************************************************

// ********** Begin Class UHktClientCoreSubsystem **************************************************
void UHktClientCoreSubsystem::StaticRegisterNativesUHktClientCoreSubsystem()
{
}
FClassRegistrationInfo Z_Registration_Info_UClass_UHktClientCoreSubsystem;
UClass* UHktClientCoreSubsystem::GetPrivateStaticClass()
{
	using TClass = UHktClientCoreSubsystem;
	if (!Z_Registration_Info_UClass_UHktClientCoreSubsystem.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			StaticPackage(),
			TEXT("HktClientCoreSubsystem"),
			Z_Registration_Info_UClass_UHktClientCoreSubsystem.InnerSingleton,
			StaticRegisterNativesUHktClientCoreSubsystem,
			sizeof(TClass),
			alignof(TClass),
			TClass::StaticClassFlags,
			TClass::StaticClassCastFlags(),
			TClass::StaticConfigName(),
			(UClass::ClassConstructorType)InternalConstructor<TClass>,
			(UClass::ClassVTableHelperCtorCallerType)InternalVTableHelperCtorCaller<TClass>,
			UOBJECT_CPPCLASS_STATICFUNCTIONS_FORCLASS(TClass),
			&TClass::Super::StaticClass,
			&TClass::WithinClass::StaticClass
		);
	}
	return Z_Registration_Info_UClass_UHktClientCoreSubsystem.InnerSingleton;
}
UClass* Z_Construct_UClass_UHktClientCoreSubsystem_NoRegister()
{
	return UHktClientCoreSubsystem::GetPrivateStaticClass();
}
struct Z_Construct_UClass_UHktClientCoreSubsystem_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "IncludePath", "HktClientCoreSubsystem.h" },
		{ "ModuleRelativePath", "Public/HktClientCoreSubsystem.h" },
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UHktClientCoreSubsystem>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_UHktClientCoreSubsystem_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UGameInstanceSubsystem,
	(UObject* (*)())Z_Construct_UPackage__Script_HktClient,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UHktClientCoreSubsystem_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UHktClientCoreSubsystem_Statics::ClassParams = {
	&UHktClientCoreSubsystem::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	nullptr,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	0,
	0,
	0x001000A0u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UHktClientCoreSubsystem_Statics::Class_MetaDataParams), Z_Construct_UClass_UHktClientCoreSubsystem_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UHktClientCoreSubsystem()
{
	if (!Z_Registration_Info_UClass_UHktClientCoreSubsystem.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UHktClientCoreSubsystem.OuterSingleton, Z_Construct_UClass_UHktClientCoreSubsystem_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UHktClientCoreSubsystem.OuterSingleton;
}
UHktClientCoreSubsystem::UHktClientCoreSubsystem() {}
DEFINE_VTABLE_PTR_HELPER_CTOR(UHktClientCoreSubsystem);
UHktClientCoreSubsystem::~UHktClientCoreSubsystem() {}
// ********** End Class UHktClientCoreSubsystem ****************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktClientCoreSubsystem_h__Script_HktClient_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UHktClientCoreSubsystem, UHktClientCoreSubsystem::StaticClass, TEXT("UHktClientCoreSubsystem"), &Z_Registration_Info_UClass_UHktClientCoreSubsystem, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UHktClientCoreSubsystem), 3281173370U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktClientCoreSubsystem_h__Script_HktClient_193580925(TEXT("/Script/HktClient"),
	Z_CompiledInDeferFile_FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktClientCoreSubsystem_h__Script_HktClient_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktClientCoreSubsystem_h__Script_HktClient_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
