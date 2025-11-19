// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassRepresentationTrait.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassRepresentationFragments.h"
#include "MassRepresentationSubsystem.h"
#include "MassLODFragments.h"

void UHktMassRepresentationTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FTransformFragment>();
	
	// Representation Fragment 추가
	FMassRepresentationFragment& RepresentationFragment = BuildContext.AddFragment_GetRef<FMassRepresentationFragment>();
	UMassRepresentationSubsystem* RepresentationSubsystem = UWorld::GetSubsystem<UMassRepresentationSubsystem>(&World);
	if (RepresentationSubsystem)
	{
		RepresentationFragment.CurrentRepresentation = EMassRepresentationType::StaticMeshInstance;
		RepresentationFragment.StaticMeshDescHandle = RepresentationSubsystem->FindOrAddStaticMeshDesc(MeshDesc);
	}

	// Representation LOD Fragment
	BuildContext.AddFragment<FMassRepresentationLODFragment>();
	
	// Representation Subsystem Shared Fragment
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	FMassRepresentationSubsystemSharedFragment RepresentationSharedFragment;
	RepresentationSharedFragment.RepresentationSubsystem = RepresentationSubsystem;
	FSharedStruct SubsystemFragment = EntityManager.GetOrCreateSharedFragment(RepresentationSharedFragment);
	BuildContext.AddSharedFragment(SubsystemFragment);
}

