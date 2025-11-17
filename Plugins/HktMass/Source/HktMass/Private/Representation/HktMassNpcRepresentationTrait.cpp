// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassNpcRepresentationTrait.h"
#include "MassEntityTemplateRegistry.h"
#include "MassRepresentationFragments.h"
#include "MassRepresentationSubsystem.h"
#include "MassLODFragments.h"

void UHktMassNpcRepresentationTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	// Representation Fragment ì¶”ê?
	FMassRepresentationFragment& RepresentationFragment = BuildContext.AddFragment_GetRef<FMassRepresentationFragment>();
	UMassRepresentationSubsystem* RepresentationSubsystem = UWorld::GetSubsystem<UMassRepresentationSubsystem>(&World);
	if (RepresentationSubsystem)
	{
		RepresentationFragment.CurrentRepresentation = EMassRepresentationType::StaticMeshInstance;
		RepresentationFragment.StaticMeshDescHandle = RepresentationSubsystem->FindOrAddStaticMeshDesc(NpcMeshDesc);
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

