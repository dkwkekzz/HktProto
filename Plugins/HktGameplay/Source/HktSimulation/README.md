HktSimulation - 자연어 스타일 바이트코드 VM
결정론적 멀티플레이어 시뮬레이션을 위한 경량 바이트코드 VM입니다. GAS의 복잡성 없이 자연어처럼 읽히는 선형적 스킬/행동 로직을 처리합니다.
핵심 설계 원칙
자연어 시간-공간 연속성: 콜백 없이 위에서 아래로 선형으로 읽힘
결정론적 실행: 서버/클라이언트 동일 결과 보장
캐시 친화적 메모리: SOA 레이아웃
Flow 예제
파이어볼 스킬
// "시전 애니메이션을 재생하고 1초 기다린다.
//  파이어볼을 생성하여 앞으로 날린다.
//  충돌하면 파이어볼을 제거하고 직격 대상에게 100 피해를 준다.
//  주변 300 범위 내 대상들에게 각각 50 피해와 화상을 입힌다."

Flow("Ability.Skill.Fireball")
    .PlayAnim(Self, "CastFireball")
    .WaitSeconds(1.0f)
    
    .SpawnEntity("/Game/Projectiles/BP_Fireball")
    .GetPosition(R0, Self)
    .SetPosition(Spawned, R0)
    .MoveForward(Spawned, 500)
    
    .WaitCollision(Spawned)
    
    .GetPosition(R3, Spawned)
    .DestroyEntity(Spawned)
    .ApplyDamageConst(Hit, 100)
    .PlayVFX(R3, "/Game/VFX/FireballExplosion")
    
    .ForEachInRadius(Hit, 300)
        .Move(Target, Iter)
        .ApplyDamageConst(Target, 50)
        .ApplyEffect(Target, "Effect.Burn")
    .EndForEach()
    .BuildAndRegister();

위치 이동
// "목표 위치로 이동하고, 도착하면 정지한다."

Flow("Action.Move.ToLocation")
    .LoadStore(R0, PropertyId::TargetPosX)
    .LoadStore(R1, PropertyId::TargetPosY)
    .LoadStore(R2, PropertyId::TargetPosZ)
    
    .PlayAnim(Self, "Run")
    .MoveToward(Self, R0, 300)
    .WaitMoveEnd(Self)
    
    .StopMovement(Self)
    .PlayAnim(Self, "Idle")
    .BuildAndRegister();

캐릭터 입장
// "캐릭터를 생성하고 스폰 애니메이션을 재생한다.
//  0.5초 후 장비를 생성하고 인트로 애니메이션을 재생한다."

Flow("Event.Character.Spawn")
    .SpawnEntity("/Game/Characters/BP_PlayerCharacter")
    .Move(Self, Spawned)
    
    .LoadStore(R0, PropertyId::TargetPosX)
    .LoadStore(R1, PropertyId::TargetPosY)
    .LoadStore(R2, PropertyId::TargetPosZ)
    .SetPosition(Self, R0)
    
    .PlayAnim(Self, "Spawn")
    .WaitSeconds(0.5f)
    
    .SpawnEquipment(Self, 0, "/Game/Weapons/BP_Sword")
    .SpawnEquipment(Self, 1, "/Game/Equipment/BP_Shield")
    
    .PlayAnimMontage(Self, "IntroMontage")
    .WaitAnimEnd(Self)
    
    .PlayAnim(Self, "Idle")
    .BuildAndRegister();

아키텍처
┌────────────────────────────────────────────────────────────┐
│                      VMProcessor                            │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐              │
│  │  Build   │───>│ Execute  │───>│ Cleanup  │              │
│  │          │    │          │    │          │              │
│  │ Intent   │    │ VM들을   │    │ Store    │              │
│  │ Event    │    │ yield    │    │ 변경사항 │              │
│  │ → VM     │    │ 까지     │    │ 적용     │              │
│  └──────────┘    └──────────┘    └──────────┘              │
└────────────────────────────────────────────────────────────┘

레지스터 규약
레지스터
용도
R0-R7
범용
Self (R8)
실행 주체 EntityId
Target (R9)
현재 타겟 EntityId
Iter (R10)
ForEach 이터레이터
Count (R11)
검색 결과 수
Temp (R12)
임시
Flag (R13)
조건 플래그
Spawned (R14)
마지막 스폰된 EntityId
Hit (R15)
충돌 대상 EntityId
OpCode 카테고리
Control Flow
Yield, YieldSeconds, WaitCollision, WaitAnimEnd, WaitMoveEnd
Jump, JumpIf, JumpIfNot, Halt
Entity
SpawnEntity, DestroyEntity
GetPosition, SetPosition
MoveToward, MoveForward, StopMovement
Spatial Query
FindInRadius, NextFound
ForEach 패턴: ForEachInRadius() ... EndForEach()
Combat
ApplyDamage, ApplyEffect, RemoveEffect
Animation & VFX
PlayAnim, PlayAnimMontage, StopAnim
PlayVFX, PlayVFXAttached
PlaySound, PlaySoundAtLocation
Equipment
SpawnEquipment
파일 구조
HktSimulation/
├── Source/HktSimulation/
│   ├── Public/
│   │   ├── VMTypes.h          # 타입, OpCode, 레지스터 정의
│   │   ├── VMProgram.h        # Program + FlowBuilder
│   │   ├── VMStore.h          # Store + StashComponent
│   │   ├── VMRuntime.h        # Runtime + Pool
│   │   ├── VMProcessor.h      # 3단계 파이프라인
│   │   ├── VMInterpreter.h    # 바이트코드 실행
│   │   └── FlowDefinitions.h  # 예제 Flow들
│   ├── Private/
│   │   └── *.cpp
│   └── HktSimulation.Build.cs
└── README.md

사용법
// 1. Flow 등록 (자동으로 처리됨)
// SimulationManager::BeginPlay에서 FlowDefinitions::RegisterAllFlows() 호출

// 2. GameState에서 초기화 (서버)
void AMyGameState::BeginPlay()
{
    Super::BeginPlay();
    
    if (HasAuthority())
    {
        // Stash 생성
        Stash = NewObject<UStashComponent>(this);
        Stash->RegisterComponent();
        
        // 초기 엔티티들 생성
        EntityId Player1 = Stash->AllocateEntity();
        Stash->SetProperty(Player1, PropertyId::Health, 100);
        Stash->SetProperty(Player1, PropertyId::PosX, 0);
        // ...
        
        // SimulationManager 초기화
        SimManager = NewObject<USimulationManager>(this);
        SimManager->RegisterComponent();
        SimManager->InitializeAsServer(Stash);
        
        // 게임 시작 (클라이언트에게 스냅샷 전송)
        SimManager->StartSimulation();
    }
}

// 3. 플레이어 입력 처리
void AMyPlayerController::UseSkill(FGameplayTag SkillTag, FVector TargetPos)
{
    FIntentEvent Event;
    Event.SourceEntity = MyEntityId;
    Event.EventTag = SkillTag;
    Event.TargetLocation = TargetPos;
    
    // 서버로 전송 → 서버가 모든 클라이언트에 브로드캐스트
    Server_QueueIntent(Event);
}

// 4. 게임 시스템에서 이벤트 알림
// 물리 충돌 감지 시:
SimManager->GetProcessor()->NotifyCollision(FireballEntity, HitEntity);

// 애니메이션 종료 시:
SimManager->GetProcessor()->NotifyAnimEnd(PlayerEntity);

네트워크 동기화
초기 동기화 (스냅샷)
┌─────────┐                      ┌─────────┐
│  Server │                      │ Client  │
├─────────┤                      ├─────────┤
│ 1. 초기 엔티티 생성            │         │
│ 2. StartSimulation()           │         │
│    └─► CreateSnapshot()        │         │
│    └─► BroadcastSnapshot() ──────────────► ClientReceiveSnapshot()
│                                │    └─► RestoreFromSnapshot()
│ 3. 시뮬레이션 시작             │ 4. 시뮬레이션 시작
└─────────┘                      └─────────┘

런타임 (결정론적 실행)
서버/클라이언트 모두 동일한 로직 실행:
  IntentEvent(Frame N) → VMProcessor.Tick() → Stash 업데이트
  
체크섬 검증 (설정된 주기마다):
  서버: MulticastValidateChecksum(Frame, Checksum)
  클라이언트: 로컬 체크섬 비교 → 불일치 시 ServerReportDesync()

디싱크 복구
// Stash에 디싱크 핸들러 등록
Stash->OnDesyncDetected.AddLambda([this](int32 Frame, uint32 Expected, uint32 Actual)
{
    UE_LOG(LogTemp, Error, TEXT("Desync at frame %d!"), Frame);
    // 옵션 1: 재동기화 요청
    // 옵션 2: 게임 중단
});


