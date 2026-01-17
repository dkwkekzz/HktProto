#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "HktIntentInterfaces.h"
#include "HktIntentGroupManager.generated.h"

/**
 * 인텐트 이벤트를 기반으로 유닛들의 그룹을 관리하는 서브시스템
 */
UCLASS()
class UHktIntentGroupManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // 이벤트를 추가하고 그룹을 갱신합니다.
    void AddIntent(const FHktIntentEvent& InEvent);

    // 이벤트를 제거하고 필요 시 그룹을 분리합니다.
    void RemoveIntent(const FHktIntentEvent& InEvent);

    // 특정 유닛이 속한 그룹의 멤버들을 가져옵니다.
    const TSet<FHktUnitHandle>* GetGroupMembers(const FHktUnitHandle& Unit) const;

    // 디버깅용: 현재 그룹 상태를 로그로 출력
    void DebugPrintGroups();

private:
    // 그룹 ID 발급용 카운터
    int32 NextGroupID = 0;

    // 유닛 핸들 -> 그룹 ID 매핑
    TMap<FHktUnitHandle, int32> UnitToGroupIDMap;

    // 그룹 ID -> 그룹 데이터 매핑
    TMap<int32, FHktIntentEventGroup> Groups;

    // 인접 리스트 (그래프 구조): A -> {B: 연결수, C: 연결수...}
    // 동일한 쌍에 대해 여러 이벤트가 있을 수 있으므로 참조 카운팅(Ref Count)을 사용합니다.
    TMap<FHktUnitHandle, TMap<FHktUnitHandle, int32>> AdjacencyGraph;

private:
    // 두 유닛 사이에 간선(Edge) 추가
    void AddEdge(const FHktUnitHandle& NodeA, const FHktUnitHandle& NodeB);
    
    // 두 유닛 사이의 간선 제거
    void RemoveEdge(const FHktUnitHandle& NodeA, const FHktUnitHandle& NodeB);

    // 두 그룹을 병합 (Merge)
    void MergeGroups(int32 GroupIdKeep, int32 GroupIdAbsorb);

    // 그룹 내 연결성 확인 및 분리 (Split)
    // StartNode가 속한 그룹이 쪼개졌는지 확인하고 재구성합니다.
    void CheckAndSplitGroup(int32 GroupID, const FHktUnitHandle& StartNode);

    // BFS를 사용하여 연결된 컴포넌트 탐색
    void FindConnectedComponent(const FHktUnitHandle& StartNode, TSet<FHktUnitHandle>& OutComponent);
};