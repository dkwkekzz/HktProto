// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "HktDataAsset.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void EmptyLinkFunctionForGeneratedCodeHktDataAsset() {}

// ********** Begin Cross Module References ********************************************************
ENGINE_API UClass* Z_Construct_UClass_UPrimaryDataAsset();
HKTCLIENT_API UClass* Z_Construct_UClass_UHktDataAsset();
HKTCLIENT_API UClass* Z_Construct_UClass_UHktDataAsset_NoRegister();
UPackage* Z_Construct_UPackage__Script_HktClient();
// ********** End Cross Module References **********************************************************

// ********** Begin Class UHktDataAsset ************************************************************
void UHktDataAsset::StaticRegisterNativesUHktDataAsset()
{
}
FClassRegistrationInfo Z_Registration_Info_UClass_UHktDataAsset;
UClass* UHktDataAsset::GetPrivateStaticClass()
{
	using TClass = UHktDataAsset;
	if (!Z_Registration_Info_UClass_UHktDataAsset.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			StaticPackage(),
			TEXT("HktDataAsset"),
			Z_Registration_Info_UClass_UHktDataAsset.InnerSingleton,
			StaticRegisterNativesUHktDataAsset,
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
	return Z_Registration_Info_UClass_UHktDataAsset.InnerSingleton;
}
UClass* Z_Construct_UClass_UHktDataAsset_NoRegister()
{
	return UHktDataAsset::GetPrivateStaticClass();
}
struct Z_Construct_UClass_UHktDataAsset_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "IncludePath", "HktDataAsset.h" },
		{ "ModuleRelativePath", "Public/HktDataAsset.h" },
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UHktDataAsset>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_UHktDataAsset_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UPrimaryDataAsset,
	(UObject* (*)())Z_Construct_UPackage__Script_HktClient,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UHktDataAsset_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UHktDataAsset_Statics::ClassParams = {
	&UHktDataAsset::StaticClass,
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
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UHktDataAsset_Statics::Class_MetaDataParams), Z_Construct_UClass_UHktDataAsset_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UHktDataAsset()
{
	if (!Z_Registration_Info_UClass_UHktDataAsset.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UHktDataAsset.OuterSingleton, Z_Construct_UClass_UHktDataAsset_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UHktDataAsset.OuterSingleton;
}
UHktDataAsset::UHktDataAsset(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(UHktDataAsset);
UHktDataAsset::~UHktDataAsset() {}
// ********** End Class UHktDataAsset **************************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktDataAsset_h__Script_HktClient_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UHktDataAsset, UHktDataAsset::StaticClass, TEXT("UHktDataAsset"), &Z_Registration_Info_UClass_UHktDataAsset, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UHktDataAsset), 66045887U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktDataAsset_h__Script_HktClient_1308450965(TEXT("/Script/HktClient"),
	Z_CompiledInDeferFile_FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktDataAsset_h__Script_HktClient_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_UE5_HktProto_Plugins_HktClient_Source_HktClient_Public_HktDataAsset_h__Script_HktClient_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
