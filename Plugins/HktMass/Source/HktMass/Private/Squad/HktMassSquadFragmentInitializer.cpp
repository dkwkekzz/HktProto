// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassSquadFragmentInitializer.h"
#include "HktMassSquadFragments.h"
#include "MassSpawnerSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassCommonFragments.h"
#include "MassEntityTypes.h"
#include "MassExecutionContext.h"

UHktMassSquadFragmentInitializer::UHktMassSquadFragmentInitializer()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	// SquadFragment가 추가될 때 실행
	ObservedType = FHktMassSquadFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void UHktMassSquadFragmentInitializer::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FHktMassSquadFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
}

void UHktMassSquadFragmentInitializer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [&](FMassExecutionContext& ChunkContext)
	{
		const TArrayView<FHktMassSquadFragment> SquadFragments = ChunkContext.GetMutableFragmentView<FHktMassSquadFragment>();
		const TConstArrayView<FTransformFragment> Transforms = ChunkContext.GetFragmentView<FTransformFragment>();

		for (int32 i = 0; i < ChunkContext.GetNumEntities(); ++i)
		{
			FHktMassSquadFragment& SquadFrag = SquadFragments[i];
			const FTransform& SquadTransform = Transforms[i].GetTransform();
			const FMassEntityHandle SquadEntity = ChunkContext.GetEntity(i);

			if (SquadFrag.MemberConfig && SquadFrag.MemberCount > 0)
			{
				// Entity 스폰은 즉시 실행하지 않고 Deferred Command로 처리
				ChunkContext.Defer().PushCommand<FMassDeferredCreateCommand>(
					[SquadEntity, MemberConfig = SquadFrag.MemberConfig, MemberCount = SquadFrag.MemberCount, SquadMaxRadius = SquadFrag.SquadMaxRadius, SquadTransform](FMassEntityManager& Manager)
					{
						UWorld* World = Manager.GetWorld();
						if (!World) return;

						UMassSpawnerSubsystem* SpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
						if (!SpawnerSubsystem) return;

						const FMassEntityTemplate& SquadEntityTemplate = MemberConfig->GetOrCreateEntityTemplate(*World);
						if (!SquadEntityTemplate.IsValid()) return;

						TArray<FMassEntityHandle> SpawnedMembers;
						SpawnerSubsystem->SpawnEntities(SquadEntityTemplate, MemberCount, SpawnedMembers);

						// 스폰된 멤버 정보 업데이트 (SquadFragment에 저장)
						if (FHktMassSquadFragment* CurrentSquadFrag = Manager.GetFragmentDataPtr<FHktMassSquadFragment>(SquadEntity))
						{
							CurrentSquadFrag->MemberEntities = SpawnedMembers;
						}

						// 멤버 초기화
						for (int32 m = 0; m < SpawnedMembers.Num(); ++m)
						{
							FMassEntityHandle MemberHandle = SpawnedMembers[m];

							float Angle = (2.0f * PI * m) / MemberCount;
							float Radius = 200.0f; // 2m 반경
							FVector Offset = FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);

							// Squad Member Fragment 설정
							if (FHktMassSquadMemberFragment* MemberFrag = Manager.GetFragmentDataPtr<FHktMassSquadMemberFragment>(MemberHandle))
							{
								// SquadEntity Handle 직접 저장
								MemberFrag->ParentSquadEntity = SquadEntity;
								MemberFrag->FormationOffset = Offset;
								MemberFrag->MaxOffset = SquadMaxRadius;
							}

							// 초기 위치 설정
							if (FTransformFragment* MemberTransformFrag = Manager.GetFragmentDataPtr<FTransformFragment>(MemberHandle))
							{
								FTransform MemberTransform = SquadTransform;
								MemberTransform.AddToTranslation(Offset);
								MemberTransformFrag->SetTransform(MemberTransform);
							}
						}
					}
				);
			}
		}
	});
}
