# HktProto Mass Entity - 리팩토링된 구조

## 📁 폴더 구조 (관심사 분리)

```
MassEntity/
├── Core/                               공통 Fragment 및 서브시스템
│   ├── HktMassNpcFragments.h/cpp      모든 NPC Fragment 정의
│   └── HktMassNpcManagerSubsystem.h/cpp
│
├── Movement/                           이동 관련
│   ├── HktMassNpcMovementTrait.h/cpp  이동 Trait
│   └── HktMassNpcMovementProcessor.h/cpp
│
├── Combat/                             전투 관련
│   └── HktMassNpcCombatTrait.h/cpp    전투 Trait
│
├── AI/                                 AI 상태 관리
│   ├── HktMassNpcAITrait.h/cpp        AI Trait
│   └── HktMassNpcAIProcessor.h/cpp
│
├── Animation/                          애니메이션
│   ├── HktMassNpcAnimationTrait.h/cpp
│   ├── HktMassNpcAnimationTypes.h
│   ├── HktMassNpcAnimationProcessor.h/cpp
│   └── HktMassNpcUpdateISMBoneAnimationProcessor.h/cpp
│
├── Representation/                     시각화
│   └── HktMassNpcRepresentationTrait.h/cpp
│
├── Replication/                        네트워크 복제
│   ├── HktMassNpcReplicationTypes.h
│   ├── HktMassNpcReplicationHelpers.h/cpp
│   ├── HktMassNpcClientBubbleInfo.h/cpp
│   └── HktMassNpcReplicator.h/cpp
│
└── HktMassNpcTrait.h/cpp              ⚠️ Legacy (Deprecated)
```

## 🎯 새로운 사용 방법

### EntityConfig에서 Trait 조합

**이전 방식 (❌ Deprecated):**
```
Traits:
  [0] Hkt Melee Npc Trait (Legacy)  ← 모든 기능이 한번에
```

**새로운 방식 (✅ Recommended):**
```
Traits:
  [0] Hkt Npc Movement              ← 이동 기능
  [1] Hkt Npc Combat                ← 전투 기능
  [2] Hkt Npc AI                    ← AI 기능
  [3] Hkt Npc Animation             ← 애니메이션
  [4] Hkt Npc Representation        ← 시각화
  [5] Replication                   ← 네트워크 복제 (선택)
```

## 💡 장점

### 1. **관심사 분리 (Separation of Concerns)**
- 각 Trait가 하나의 책임만 가짐
- 코드 이해와 유지보수가 쉬움

### 2. **재사용성**
- 다른 엔티티 타입에도 Trait 조합 가능
- 예: Vehicle에 Movement + Representation만 추가

### 3. **유연성**
- 필요한 기능만 선택적으로 추가
- 예: AI 없는 정적 장애물 = Representation만

### 4. **테스트 용이성**
- 각 Trait를 독립적으로 테스트 가능
- 버그 격리가 쉬움

## 📋 Trait별 포함 Fragment

| Trait | 추가되는 Fragment |
|-------|-------------------|
| **Movement** | FTransformFragment, FMassVelocityFragment, FHktNpcMovementFragment |
| **Combat** | FHktNpcCombatFragment, FHktNpcTargetFragment |
| **AI** | FHktNpcStateFragment, FHktNpcTypeFragment, FHktNpcPatrolFragment |
| **Animation** | FHktMassNpcAnimationFragment |
| **Representation** | FMassRepresentationFragment, FMassRepresentationLODFragment |
| **Replication** | FMassNetworkIDFragment, FMassReplicatedAgentFragment, 기타 복제 Fragment |

## 🔧 EntityConfig 설정 예시

### Melee NPC (근접 전투 유닛)
```
Traits:
  [0] Hkt Npc Movement
      Max Speed: 350.0
      Initial Speed Ratio: 0.8
      
  [1] Hkt Npc Combat
      Max Health: 150.0
      Attack Power: 20.0
      Attack Range: 100.0
      Attack Cooldown: 1.5
      
  [2] Hkt Npc AI
      Npc Type: 0 (Melee)
      Team Id: 0
      Patrol Radius: 500.0
      
  [3] Hkt Npc Animation
      Anim To Texture Data: (에셋 선택)
      
  [4] Hkt Npc Representation
      Npc Mesh Desc: (메시 설정)
      
  [5] Replication (멀티플레이어용)
      Params:
        Bubble Info Class: AHktMassNpcClientBubbleInfo
        Replicator Class: UHktMassNpcReplicator
        LOD Distance: [1000, 3000, 6000, 10000]
```

### Ranged NPC (원거리 공격 유닛)
```
Traits:
  [0] Hkt Npc Movement
      Max Speed: 300.0
      
  [1] Hkt Npc Combat
      Max Health: 80.0
      Attack Power: 15.0
      Attack Range: 500.0
      
  [2~5] 동일...
```

### Tank NPC (탱커 유닛)
```
Traits:
  [0] Hkt Npc Movement
      Max Speed: 250.0
      
  [1] Hkt Npc Combat
      Max Health: 300.0
      Attack Power: 10.0
      Attack Range: 100.0
      
  [2~5] 동일...
```

### 정적 장애물 (AI 없음)
```
Traits:
  [0] Hkt Npc Representation  ← 시각화만
```

### 이동하는 장애물 (AI 없음)
```
Traits:
  [0] Hkt Npc Movement        ← 이동만
  [1] Hkt Npc Representation  ← 시각화
```

## 🔄 Migration Guide (마이그레이션 가이드)

기존 EntityConfig를 새 구조로 변경:

### 1단계: Legacy Trait 제거
```
❌ 제거: Hkt Melee Npc Trait (Legacy)
```

### 2단계: 개별 Trait 추가
```
✅ 추가:
  - Hkt Npc Movement
  - Hkt Npc Combat
  - Hkt Npc AI
  - Hkt Npc Animation
  - Hkt Npc Representation
  - Replication (필요시)
```

### 3단계: 각 Trait 설정
기존 Legacy Trait의 프로퍼티 값을 새 Trait들에 분배

## 📚 참고

- **Processor 순서**: Movement → AI → Animation → Representation → Replication
- **Fragment 의존성**: Processor가 필요한 Fragment는 해당 Trait가 추가해야 함
- **성능**: Trait 추가는 컴파일 타임에 처리되므로 런타임 오버헤드 없음

## 🐛 문제 해결

### "Fragment X를 찾을 수 없습니다"
→ 해당 Fragment를 추가하는 Trait가 누락되었습니다.
→ 예: FHktNpcMovementFragment → Hkt Npc Movement Trait 추가

### "Processor가 실행되지 않습니다"
→ Processor가 요구하는 모든 Fragment가 있는지 확인
→ 예: MovementProcessor는 Movement + Target + State Fragment 필요

### Legacy Trait 사용 시 경고
→ 새 구조로 마이그레이션 권장
→ Legacy Trait는 향후 버전에서 제거될 예정

