// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "Collision/HktMassCollisionProcessor.h"
#include "HktMassCollisionFragments.h"
#include "HktMassMovementFragments.h"
#include "MassCommonFragments.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"

// Helper class for simple spatial hashing
struct FMassEntityHashGrid2D
{
	struct FItem
	{
		FMassEntityHandle Entity;
		FVector2D Location; // Storing location for distance check
	};

	float CellSize;
	TMap<FIntPoint, TArray<FItem>> Grid;

	void Initialize(float InCellSize)
	{
		CellSize = InCellSize;
		Grid.Empty();
	}

	void AddPoint(FMassEntityHandle Entity, const FVector& Location)
	{
		FVector2D Loc2D(Location.X, Location.Y);
		FIntPoint Cell(FMath::FloorToInt(Loc2D.X / CellSize), FMath::FloorToInt(Loc2D.Y / CellSize));
		Grid.FindOrAdd(Cell).Add({Entity, Loc2D});
	}

	void Query(const FBox2D& QueryBox, TArray<FItem>& OutItems) const
	{
		int32 MinX = FMath::FloorToInt(QueryBox.Min.X / CellSize);
		int32 MinY = FMath::FloorToInt(QueryBox.Min.Y / CellSize);
		int32 MaxX = FMath::FloorToInt(QueryBox.Max.X / CellSize);
		int32 MaxY = FMath::FloorToInt(QueryBox.Max.Y / CellSize);

		for (int32 X = MinX; X <= MaxX; ++X)
		{
			for (int32 Y = MinY; Y <= MaxY; ++Y)
			{
				if (const TArray<FItem>* CellItems = Grid.Find(FIntPoint(X, Y)))
				{
					for (const FItem& Item : *CellItems)
					{
						if (QueryBox.IsInside(Item.Location))
						{
							OutItems.Add(Item);
						}
					}
				}
			}
		}
	}
};

UHktMassCollisionProcessor::UHktMassCollisionProcessor()
{
	// 물리 처리 이전에 속도를 수정해야 하므로 PrePhysics 혹은 Avoidance 단계가 적절
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	bAutoRegisterWithProcessingPhases = true;
}

void UHktMassCollisionProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// 필요한 컴포넌트 정의
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHktMassVelocityFragment>(EMassFragmentAccess::ReadWrite); // 속도를 수정하여 밀어냄
	EntityQuery.AddRequirement<FHktMassCollisionFragment>(EMassFragmentAccess::ReadOnly);
	
	// LOD 처리를 위한 Chunk Requirement
	// EntityQuery.AddRequirement<FMassSimulationVariableTickFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	// EntityQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	// EntityQuery.SetChunkFilter(&FMassSimulationVariableTickChunkFragment::ShouldTickChunkThisFrame);
}

void UHktMassCollisionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// 1. 공간 분할(Hash Grid) 구축 단계
	// 간단한 구현을 위해 로컬 해시 그리드 생성 (Cell Size: 100cm)
	FMassEntityHashGrid2D HashGrid;
	HashGrid.Initialize(100.0f);

	// 1-A. 모든 엔티티를 순회하며 HashGrid에 등록
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [&HashGrid](FMassExecutionContext& LoopContext)
	{
		const int32 NumEntities = LoopContext.GetNumEntities();
		const auto& Transforms = LoopContext.GetFragmentView<FTransformFragment>();
		const auto& Collisions = LoopContext.GetFragmentView<FHktMassCollisionFragment>();

		for (int32 i = 0; i < NumEntities; ++i)
		{
			const FVector Pos = Transforms[i].GetTransform().GetLocation();
			// 엔티티 핸들과 위치를 그리드에 추가
			HashGrid.AddPoint(LoopContext.GetEntity(i), Pos);
		}
	});

	// 2. 충돌 감지 및 해결 (Repulsion & Event Tagging)
	// DeltaTime 가져오기
	const float DeltaTime = Context.GetDeltaTimeSeconds();

	EntityQuery.ForEachEntityChunk(EntityManager, Context, [&HashGrid, &EntityManager, DeltaTime](FMassExecutionContext& LoopContext)
	{
		const int32 NumEntities = LoopContext.GetNumEntities();
		
		// 데이터 뷰 가져오기
		TConstArrayView<FTransformFragment> Transforms = LoopContext.GetFragmentView<FTransformFragment>();
		TArrayView<FHktMassVelocityFragment> Velocities = LoopContext.GetMutableFragmentView<FHktMassVelocityFragment>();
		TConstArrayView<FHktMassCollisionFragment> Collisions = LoopContext.GetFragmentView<FHktMassCollisionFragment>();

		// 검색 결과를 담을 배열 (재사용)
		TArray<FMassEntityHashGrid2D::FItem> CloseEntities;

		for (int32 i = 0; i < NumEntities; ++i)
		{
			const FVector CurrentPos = Transforms[i].GetTransform().GetLocation();
			const float MyRadius = Collisions[i].Radius;
			const float RepulsionStrength = Collisions[i].RepulsionForce;
			FMassEntityHandle MyEntity = LoopContext.GetEntity(i);

			// 내 주변 (Radius * 2) 범위 내의 엔티티 검색
			const float SearchRadius = MyRadius * 2.5f; // 약간 여유 있게 검색
			FVector2D SearchCenter(CurrentPos.X, CurrentPos.Y);
			FBox2D QueryBox(SearchCenter - FVector2D(SearchRadius), SearchCenter + FVector2D(SearchRadius));

			CloseEntities.Reset();
			HashGrid.Query(QueryBox, CloseEntities);

			FVector TotalRepulsionForce = FVector::ZeroVector;
			bool bHasCollision = false;

			for (const auto& Item : CloseEntities)
			{
				// 나 자신은 제외
				if (Item.Entity == MyEntity) continue;

				// Item.Location (HashGrid에 저장된 위치)을 활용
				FVector OtherPos(Item.Location.X, Item.Location.Y, CurrentPos.Z); // 높이는 같다고 가정

				// 거리 계산 (2D 평면 기준)
				FVector ToMe = CurrentPos - OtherPos;
				ToMe.Z = 0.0f; // 높이 무시
				
				float DistSq = ToMe.SizeSquared();
				float MinDist = MyRadius + MyRadius; // 상대방 반지름도 같다 가정 (다르다면 룩업 필요)
				
				if (DistSq > KINDA_SMALL_NUMBER && DistSq < (MinDist * MinDist))
				{
					float Dist = FMath::Sqrt(DistSq);
					float Overlap = MinDist - Dist; // 겹친 정도
					
					// 밀어내는 방향
					FVector PushDir = ToMe / Dist;
					
					// 힘 누적: 겹친 만큼 강하게, Repulsion 계수 적용
					TotalRepulsionForce += PushDir * (Overlap * RepulsionStrength);
					bHasCollision = true;
				}
			}

			// 3. 결과 적용
			if (bHasCollision)
			{
				// A. 속도에 힘 적용 (F = ma -> a = F/m, 여기선 질량 1 가정)
				// 기존 속도에 가산하여 부드럽게 밀려나게 함
				Velocities[i].Value += TotalRepulsionForce * DeltaTime;

				// B. 이벤트 태그 부착 (이미 있으면 무시됨)
				LoopContext.Defer().AddTag<FHktMassCollisionHitTag>(MyEntity);
			}
			else
			{
				// 충돌이 끝났으면 태그 제거
				LoopContext.Defer().RemoveTag<FHktMassCollisionHitTag>(MyEntity);
			}
		}
	});
}

