// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "HktClientViewSubsystem.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void EmptyLinkFunctionForGeneratedCodeHktClientViewSubsystem() {}

// ********** Begin Cross Module References ********************************************************
ENGINE_API UClass* Z_Construct_UClass_UWorldSubsystem();
HKTCLIENT_API UClass* Z_Construct_UClass_UHktClientViewSubsystem();
HKTCLIENT_API UClass* Z_Construct_UClass_UHktClientViewSubsystem_NoRegister();
UPackage* Z_Construct_UPackage__Script_HktClient();
// ********** End Cross Module References **********************************************************

// ********** Begin Class UHktClientViewSubsystem **************************************************
void UHktClientViewSubsystem::StaticRegisterNativesUHktClientViewSubsystem()
{
}
FClassRegistrationInfo Z_Registration_Info_UClass_UHktClientViewSubsystem;
UClass* UHktClientViewSubsystem::GetPrivateStaticClass()
{
	using TClass = UHktClientViewSubsystem;
	if (!Z_Registration_Info_UClass_UHktClientViewSubsystem.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			StaticPackage(),
			TEXT("HktClientViewSubsystem"),
			Z_Registration_Info_UClass_UHktClientViewSubsystem.InnerSingleton,
			StaticRegisterNativesUHktClientViewSubsystem,
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
	return Z_Registration_Info_UClass_UHktClientViewSubsystem.InnerSingleton;
}
UClass* Z_Construct_UClass_UHktClientViewSubsystem_NoRegister()
{
	return UHktClientViewSubsystem::GetPrivateStaticClass();
}
struct Z_Construct_UClass_UHktClientViewSubsystem_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "IncludePath", "HktClientViewSubsystem.h" },
		{ "ModuleRelativePath", "Public/HktClientViewSubsystem.h" },
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UHktClientViewSubsystem>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_UHktClientViewSubsystem_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UWorldSubsystem,
	(UObject* (*)())Z_Construct_UPackage__Script_HktClient,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UHktClientViewSubsystem_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UHktClientViewSubsystem_Statics::ClassParams = {
	&UHktClientViewSubsystem::StaticClass,
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
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UHktClientViewSubsystem_Statics::Class_MetaDataParams), Z_Construct_UClass_UHktClientViewSubsystem_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UHktClientViewSubsystem()
{
	if (!Z_Registration_Info_UClass_UHktClientViewSubsystem.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UHktClientViewSubsystem.OuterSingleton, Z_Construct_UClass_UHktClientViewSubsystem_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UHktClientViewSubsystem.OuterSingleton;
}
UHktClientViewSubsystem::UHktClientViewSubsystem() {}
DEFINE_VTABLE_PTR_HELPER_CTOR(UHktClientViewSubsystem);
UHktClientViewSubsystem::~UHktClientViewSubsystem() {}
// ********** End Class UHktClientViewSubsystem ****************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktClientViewSubsystem_h__Script_HktClient_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UHktClientViewSubsystem, UHktClientViewSubsystem::StaticClass, TEXT("UHktClientViewSubsystem"), &Z_Registration_Info_UClass_UHktClientViewSubsystem, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UHktClientViewSubsystem), 2761967118U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktClientViewSubsystem_h__Script_HktClient_571337235(TEXT("/Script/HktClient"),
	Z_CompiledInDeferFile_FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktClientViewSubsystem_h__Script_HktClient_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktClientViewSubsystem_h__Script_HktClient_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
