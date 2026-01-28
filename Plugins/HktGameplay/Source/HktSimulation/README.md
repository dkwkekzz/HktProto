HktSimulation - 결정론적 멀티플레이어 VM
Lockstep 기반 결정론적 시뮬레이션을 위한 경량 바이트코드 VM입니다.
핵심 원리
동일한 입력 (IntentEvent) → 동일한 실행 → 동일한 결과

아키텍처
┌─────────────────────────────────────────────────────────────────────┐
│                            SERVER                                    │
│                                                                      │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │                    UMasterStashComponent                       │  │
│  │                   (모든 엔티티 데이터)                          │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                               │                                      │
│                         VMProcessor                                  │
│                               │                                      │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │                     FHktIntentEvent                            │  │
│  │  SourceEntity: 1                                               │  │
│  │  TargetEntity: 2                                               │  │
│  │  EventTag: "Ability.Skill.Fireball"                            │  │
│  │  AttachedSnapshots: [Entity 1 스냅샷, Entity 2 스냅샷]          │  │
│  │                     (클라가 모르는 것만 첨부)                    │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                               │                                      │
│                      UE Replication                                  │
└───────────────────────────────│─────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────────┐
│                            CLIENT                                    │
│                                                                      │
│  1. AttachedSnapshots 적용 (모르는 엔티티면)                         │
│  2. IntentEvent 실행 (동일한 결과)                                   │
│                                                                      │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │                   UVisibleStashComponent                       │  │
│  │              (내가 아는 엔티티 데이터만)                         │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                               │                                      │
│                         VMProcessor                                  │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘

IntentEvent 동기화 흐름
// ============================================
// SERVER: 스킬 사용 시
// ============================================
void AMyPlayerController::Server_UseSkill(FGameplayTag SkillTag, EntityId Target)
{
    FHktIntentEvent Event;
    Event.SourceEntity = MyEntityId;
    Event.TargetEntity = Target;
    Event.EventTag = SkillTag;
    
    // 클라이언트가 모르는 엔티티의 스냅샷 첨부
    for (AMyPlayerController* Client : GetRelevantClients())
    {
        FHktIntentEvent EventForClient = Event;
        
        if (!Client->KnowsEntity(Event.SourceEntity))
        {
            EventForClient.AttachedSnapshots.Add(
                MasterStash->CreateEntitySnapshot(Event.SourceEntity));
            Client->MarkEntityKnown(Event.SourceEntity);
        }
        
        if (!Client->KnowsEntity(Event.TargetEntity))
        {
            EventForClient.AttachedSnapshots.Add(
                MasterStash->CreateEntitySnapshot(Event.TargetEntity));
            Client->MarkEntityKnown(Event.TargetEntity);
        }
        
        Client->ClientReceiveIntentEvent(EventForClient);
    }
    
    // 서버도 실행
    Processor.QueueIntentEvent(Event);
}

// ============================================
// CLIENT: IntentEvent 수신
// ============================================
void AMyPlayerController::ClientReceiveIntentEvent_Implementation(
    const FHktIntentEvent& Event)
{
    // VMProcessor가 자동으로 처리:
    // 1. AttachedSnapshots 먼저 적용
    // 2. IntentEvent 실행
    Processor.QueueIntentEvent(Event);
}

핵심 컴포넌트
컴포넌트
위치
역할
UMasterStashComponent
서버
전체 엔티티 SOA 데이터
UVisibleStashComponent
클라
아는 엔티티만 SOA 데이터
IStashInterface
양쪽
Stash 추상화 인터페이스
FHktIntentEvent
양쪽
이벤트 + 필요 스냅샷
FVMProcessor
양쪽
Build → Execute → Cleanup
Flow 예제
Flow("Ability.Skill.Fireball")
    .PlayAnim(Self, "CastFireball")
    .WaitSeconds(1.0f)
    
    .SpawnEntity("/Game/Projectiles/BP_Fireball")
    .GetPosition(R0, Self)
    .SetPosition(Spawned, R0)
    .MoveForward(Spawned, 500)
    
    .WaitCollision(Spawned)
    
    .DestroyEntity(Spawned)
    .ApplyDamageConst(Hit, 100)
    
    .ForEachInRadius(Hit, 300)
        .ApplyDamageConst(Iter, 50)
        .ApplyEffect(Iter, "Effect.Burn")
    .EndForEach()
    .BuildAndRegister();

정보 은닉
클라이언트는 IntentEvent에 포함된 엔티티만 알 수 있음
시야 밖 적의 정보는 전달되지 않음
AttachedSnapshots는 서버가 필요할 때만 첨부
