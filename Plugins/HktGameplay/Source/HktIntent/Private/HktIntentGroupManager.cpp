#include "HktIntentGroupManager.h"

void UHktIntentGroupManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    NextGroupID = 1;
}

void UHktIntentGroupManager::Deinitialize()
{
    Super::Deinitialize();
}

void UHktIntentGroupManager::AddIntent(const FHktIntentEvent& InEvent)
{
    if (InEvent.Subject == InEvent.Target)
    {
        return; // 자기 자신과의 이벤트는 그룹핑에 영향 없음
    }

    AddEdge(InEvent.Subject, InEvent.Target);
}

void UHktIntentGroupManager::RemoveIntent(const FHktIntentEvent& InEvent)
{
    if (InEvent.Subject == InEvent.Target)
    {
        return;
    }

    RemoveEdge(InEvent.Subject, InEvent.Target);
}

const TSet<FHktUnitHandle>* UHktIntentGroupManager::GetGroupMembers(const FHktUnitHandle& Unit) const
{
    if (const int32* GroupID = UnitToGroupIDMap.Find(Unit))
    {
        if (const FHktIntentGroup* Group = Groups.Find(*GroupID))
        {
            return &Group->Members;
        }
    }
    return nullptr;
}

void UHktIntentGroupManager::AddEdge(const FHktUnitHandle& NodeA, const FHktUnitHandle& NodeB)
{
    // 1. 인접 그래프 업데이트 (양방향, 참조 카운트 증가)
    AdjacencyGraph.FindOrAdd(NodeA).FindOrAdd(NodeB)++;
    AdjacencyGraph.FindOrAdd(NodeB).FindOrAdd(NodeA)++;

    // 2. 그룹 상태 확인
    int32* GroupIdPtrA = UnitToGroupIDMap.Find(NodeA);
    int32* GroupIdPtrB = UnitToGroupIDMap.Find(NodeB);

    int32 GroupIdA = GroupIdPtrA ? *GroupIdPtrA : 0;
    int32 GroupIdB = GroupIdPtrB ? *GroupIdPtrB : 0;

    // Case 1: 둘 다 그룹이 없는 경우 -> 새 그룹 생성
    if (GroupIdA == 0 && GroupIdB == 0)
    {
        int32 NewID = NextGroupID++;
        FHktIntentGroup& NewGroup = Groups.Add(NewID);
        NewGroup.GroupID = NewID;
        NewGroup.Members.Add(NodeA);
        NewGroup.Members.Add(NodeB);

        UnitToGroupIDMap.Add(NodeA, NewID);
        UnitToGroupIDMap.Add(NodeB, NewID);
    }
    // Case 2: A만 그룹이 있음 -> B를 A의 그룹에 추가
    else if (GroupIdA != 0 && GroupIdB == 0)
    {
        Groups[GroupIdA].Members.Add(NodeB);
        UnitToGroupIDMap.Add(NodeB, GroupIdA);
    }
    // Case 3: B만 그룹이 있음 -> A를 B의 그룹에 추가
    else if (GroupIdA == 0 && GroupIdB != 0)
    {
        Groups[GroupIdB].Members.Add(NodeA);
        UnitToGroupIDMap.Add(NodeA, GroupIdB);
    }
    // Case 4: 둘 다 그룹이 있는데 서로 다름 -> 그룹 병합 (Union)
    else if (GroupIdA != GroupIdB)
    {
        // 더 큰 그룹쪽으로 병합하는 것이 일반적으로 효율적 (Weighted Union)
        if (Groups[GroupIdA].Members.Num() >= Groups[GroupIdB].Members.Num())
        {
            MergeGroups(GroupIdA, GroupIdB);
        }
        else
        {
            MergeGroups(GroupIdB, GroupIdA);
        }
    }
    // Case 5: 이미 같은 그룹임 -> 아무것도 안 함
}

void UHktIntentGroupManager::RemoveEdge(const FHktUnitHandle& NodeA, const FHktUnitHandle& NodeB)
{
    // 1. 인접 그래프에서 카운트 감소
    if (!AdjacencyGraph.Contains(NodeA) || !AdjacencyGraph[NodeA].Contains(NodeB))
    {
        return; // 존재하지 않는 엣지
    }

    int32& CountA = AdjacencyGraph[NodeA][NodeB];
    CountA--;

    // 대칭 노드 처리
    if (AdjacencyGraph.Contains(NodeB) && AdjacencyGraph[NodeB].Contains(NodeA))
    {
        AdjacencyGraph[NodeB][NodeA]--;
    }

    // 2. 여전히 연결되어 있다면 리턴 (다른 이벤트가 두 유닛을 묶고 있음)
    if (CountA > 0)
    {
        return;
    }

    // 3. 연결이 완전히 끊어짐 -> 그래프에서 간선 삭제
    AdjacencyGraph[NodeA].Remove(NodeB);
    AdjacencyGraph[NodeB].Remove(NodeA);
    
    // 만약 노드가 고립되었다면 인접 맵 자체에서 제거 (메모리 절약)
    if (AdjacencyGraph[NodeA].Num() == 0) AdjacencyGraph.Remove(NodeA);
    if (AdjacencyGraph[NodeB].Num() == 0) AdjacencyGraph.Remove(NodeB);

    // 4. 그룹 분리 검사 (Split Check)
    // 두 노드가 같은 그룹에 있을 때만 검사 (당연하지만 안전장치)
    int32* GroupIDPtr = UnitToGroupIDMap.Find(NodeA);
    if (GroupIDPtr)
    {
        CheckAndSplitGroup(*GroupIDPtr, NodeA);
    }
}

void UHktIntentGroupManager::MergeGroups(int32 GroupIdKeep, int32 GroupIdAbsorb)
{
    FHktIntentGroup& KeepGroup = Groups[GroupIdKeep];
    FHktIntentGroup AbsorbGroup;
    
    // Groups 맵에서 가져오고 바로 삭제하면 참조가 위험할 수 있으므로 MoveTemp 사용
    if (Groups.RemoveAndCopyValue(GroupIdAbsorb, AbsorbGroup))
    {
        for (const FHktUnitHandle& Member : AbsorbGroup.Members)
        {
            KeepGroup.Members.Add(Member);
            UnitToGroupIDMap.Add(Member, GroupIdKeep); // Map 업데이트
        }
    }
}

void UHktIntentGroupManager::CheckAndSplitGroup(int32 GroupID, const FHktUnitHandle& StartNode)
{
    if (!Groups.Contains(GroupID)) return;

    FHktIntentGroup& CurrentGroup = Groups[GroupID];
    
    // 그룹 멤버가 1명이면 더 이상 쪼갤 필요 없음 (고립됨)
    // 하지만 AdjacencyGraph에서 이미 제거되었으므로, 완전히 고립된 경우 그룹 삭제 처리
    if (CurrentGroup.Members.Num() <= 1)
    {
        // 로직상 여기까지 왔는데 멤버가 1명이면, 그 멤버는 이제 그룹이 없는 상태
        for (const FHktUnitHandle& Member : CurrentGroup.Members)
        {
            UnitToGroupIDMap.Remove(Member);
        }
        Groups.Remove(GroupID);
        return;
    }

    // BFS로 StartNode에서 도달 가능한 모든 노드를 찾음
    TSet<FHktUnitHandle> ConnectedComponent;
    FindConnectedComponent(StartNode, ConnectedComponent);

    // 만약 도달 가능한 노드 수 == 현재 그룹 전체 멤버 수라면, 그룹은 쪼개지지 않은 것임.
    if (ConnectedComponent.Num() == CurrentGroup.Members.Num())
    {
        return; 
    }

    // 그룹이 쪼개짐!
    // ConnectedComponent는 기존 GroupID를 유지하고,
    // 나머지는 새로운 그룹(들)로 분리해야 함.
    
    // 1. 기존 그룹(CurrentGroup)에서 ConnectedComponent를 뺌 = 분리되어 나간 노드들(Orphans)
    TSet<FHktUnitHandle> Orphans = CurrentGroup.Members.Difference(ConnectedComponent);
    
    // 2. 기존 그룹을 ConnectedComponent로 축소
    CurrentGroup.Members = ConnectedComponent;

    // 3. Orphans(고아 노드들) 재그룹화
    // 고아 노드들도 서로 뭉쳐있을 수도 있고, 뿔뿔이 흩어졌을 수도 있음.
    while (Orphans.Num() > 0)
    {
        // 고아 중 하나를 잡음
        FHktUnitHandle OrphanSeed = *Orphans.CreateConstIterator();
        
        // 그 고아와 연결된 덩어리를 찾음
        TSet<FHktUnitHandle> NewCluster;
        FindConnectedComponent(OrphanSeed, NewCluster);

        // 새 그룹 생성
        int32 NewGroupID = NextGroupID++;
        FHktIntentGroup& NewGroup = Groups.Add(NewGroupID);
        NewGroup.GroupID = NewGroupID;
        NewGroup.Members = NewCluster;

        // 맵핑 업데이트 및 Orphans 목록에서 제거
        for (const FHktUnitHandle& Member : NewCluster)
        {
            UnitToGroupIDMap.Add(Member, NewGroupID);
            Orphans.Remove(Member);
        }
    }
}

void UHktIntentGroupManager::FindConnectedComponent(const FHktUnitHandle& StartNode, TSet<FHktUnitHandle>& OutComponent)
{
    OutComponent.Reset();
    
    // 연결 정보가 없으면 자신만 리턴
    if (!AdjacencyGraph.Contains(StartNode))
    {
        OutComponent.Add(StartNode);
        return;
    }

    TQueue<FHktUnitHandle> Queue;
    Queue.Enqueue(StartNode);
    OutComponent.Add(StartNode);

    FHktUnitHandle Current;
    while (Queue.Dequeue(Current))
    {
        // 인접 노드 순회
        if (const TMap<FHktUnitHandle, int32>* Neighbors = AdjacencyGraph.Find(Current))
        {
            for (const auto& Pair : *Neighbors)
            {
                const FHktUnitHandle& Neighbor = Pair.Key;
                
                // 아직 방문하지 않았다면 추가
                if (!OutComponent.Contains(Neighbor))
                {
                    OutComponent.Add(Neighbor);
                    Queue.Enqueue(Neighbor);
                }
            }
        }
    }
}

void UHktIntentGroupManager::DebugPrintGroups()
{
    UE_LOG(LogTemp, Log, TEXT("=== Intent Groups State ==="));
    for (const auto& Pair : Groups)
    {
        FString MemberStr;
        for (const auto& Member : Pair.Value.Members)
        {
            MemberStr += FString::Printf(TEXT("%d, "), Member.UnitID);
        }
        UE_LOG(LogTemp, Log, TEXT("Group %d: [%s]"), Pair.Key, *MemberStr);
    }
    UE_LOG(LogTemp, Log, TEXT("==========================="));
}
