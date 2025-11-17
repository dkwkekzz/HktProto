# HktMass Plugin

Mass Entity 시스템을 활용한 고성능 NPC AI, 애니메이션, 네트워크 복제 플러그인입니다.

## 📁 프로젝트 구조

```
HktMass/
├── Source/HktMass/
│   ├── Public/                         # 공개 API
│   │   ├── Fragments/                  # Fragment 정의 (데이터 구조)
│   │   │   ├── HktMassNpcFragments.h
│   │   │   └── HktMassNpcAnimationTypes.h
│   │   ├── Types/                      # 공용 타입 정의
│   │   │   └── HktMassNpcReplicationTypes.h
│   │   ├── HktMassModule.h
│   │   └── HktMassNpcSpawnDataAsset.h
│   │
│   └── Private/                        # 내부 구현
│       ├── Fragments/                  # Fragment 구현
│       │   └── HktMassNpcFragments.cpp
│       ├── Movement/                   # 🏃 이동 시스템
│       ├── Combat/                     # ⚔️ 전투 시스템
│       ├── AI/                         # 🤖 AI 시스템
│       ├── Animation/                  # 🎬 애니메이션 시스템
│       ├── Representation/             # 🎨 시각화 시스템
│       ├── Replication/                # 🌐 네트워크 복제 시스템
│       ├── Core/                       # 🔧 핵심 서브시스템
│       └── HktMassNpcTrait.h/cpp       # Legacy (기본 NPC Trait)
```

## 🎯 핵심 개념

### Public API (공개 인터페이스)
**포함:**
- ✅ Fragment 정의 (데이터 구조)
- ✅ 공용 타입 정의
- ✅ 모듈 헤더

**특징:**
- 다른 프로젝트/모듈에서 참조 가능
- 데이터 구조만 노출, 구현은 Private에

### Private Implementation (내부 구현)
**포함:**
- ✅ Trait 구현 (엔티티 템플릿 구성)
- ✅ Processor 구현 (로직 처리)
- ✅ Fragment 구현 파일
- ✅ 내부 헬퍼 클래스

**특징:**
- 플러그인 내부 세부사항
- 외부에서 직접 접근 불가

## 📦 시스템별 구성

| 시스템 | 폴더 | Trait | Processor | Fragment |
|--------|------|-------|-----------|----------|
| **Movement** | `Movement/` | ✅ | ✅ | `FHktNpcMovementFragment` |
| **Combat** | `Combat/` | ✅ | - | `FHktNpcCombatFragment` |
| **AI** | `AI/` | ✅ | ✅ | `FHktNpcStateFragment` |
| **Animation** | `Animation/` | ✅ | ✅✅ | `FHktMassNpcAnimationFragment` |
| **Representation** | `Representation/` | ✅ | - | - |
| **Replication** | `Replication/` | - | ✅ | `FHktReplicatedNpcAgent` |
| **Core** | `Core/` | - | - | `FHktNpcTypeFragment` |

## 🔗 사용 방법

### 1. 프로젝트에 플러그인 추가

**YourProject.uproject**:
```json
{
    "Plugins": [
        {
            "Name": "HktMass",
            "Enabled": true
        }
    ]
}
```

**YourProject.Build.cs**:
```csharp
PublicDependencyModuleNames.AddRange(new string[]
{
    "HktMass"  // HktMass 플러그인 추가
});
```

### 2. Fragment 사용 예시

```cpp
// YourSystem.cpp
#include "Fragments/HktMassNpcFragments.h"

void YourSystem::UpdateNpcHealth(FMassEntityView& EntityView)
{
    // Fragment 데이터 접근
    FHktNpcCombatFragment& Combat = EntityView.GetFragmentData<FHktNpcCombatFragment>();
    Combat.CurrentHealth -= 10.0f;
    
    if (Combat.CurrentHealth <= 0.0f)
    {
        FHktNpcStateFragment& State = EntityView.GetFragmentData<FHktNpcStateFragment>();
        State.CurrentState = ENpcState::Dead;
    }
}
```

### 3. Trait를 활용한 NPC 생성

```cpp
// YourSpawner.cpp
#include "HktMassNpcSpawnDataAsset.h"

void SpawnNpc(UMassEntityConfigAsset* NpcConfig)
{
    // NpcConfig에 HktMass Trait들이 구성되어 있음:
    // - UHktMassNpcMovementTrait
    // - UHktMassNpcCombatTrait
    // - UHktMassNpcAITrait
    // - UHktMassNpcAnimationTrait
    // - UHktMassNpcRepresentationTrait
    
    UMassSpawnerSubsystem* Spawner = World->GetSubsystem<UMassSpawnerSubsystem>();
    Spawner->SpawnEntities(NpcConfig, 100, Transforms);
}
```

## 🏗️ 설계 원칙

### 1. API/Implementation Split
- **Public**: 외부에 노출되는 데이터 구조 (Fragment)
- **Private**: 내부 구현 세부사항 (Trait, Processor)
- **장점**: 컴파일 시간 단축, 의존성 최소화

### 2. Separation of Concerns
- 각 시스템(Movement, Combat, AI 등)이 독립적인 폴더
- Trait와 Processor를 컨텐츠별로 묶어 관리
- **장점**: 유지보수 용이, 코드 재사용성

### 3. Fragment-Oriented Design
- Fragment는 순수한 데이터 구조 (POD)
- 로직은 Processor에서 처리
- **장점**: 데이터 지향 설계, 캐시 친화적

## 📚 주요 Fragment

### FHktNpcTypeFragment
```cpp
struct FHktNpcTypeFragment : public FMassFragment
{
    ENpcType Type;  // Melee, Ranged, Tank
};
```

### FHktNpcMovementFragment
```cpp
struct FHktNpcMovementFragment : public FMassFragment
{
    float MoveSpeed;
    float PatrolRadius;
};
```

### FHktNpcStateFragment
```cpp
struct FHktNpcStateFragment : public FMassFragment
{
    ENpcState CurrentState;  // Idle, Patrol, Chase, Attack, Dead
    FVector TargetLocation;
};
```

### FHktNpcCombatFragment
```cpp
struct FHktNpcCombatFragment : public FMassFragment
{
    float MaxHealth;
    float CurrentHealth;
    float AttackDamage;
    float AttackRange;
};
```

### FHktMassNpcAnimationFragment
```cpp
struct FHktMassNpcAnimationFragment : public FMassFragment
{
    UAnimToTextureDataAsset* AnimToTextureData;
    int32 GlobalStartIndex;
    uint8 AnimationStateIndex;
};
```

## 🌐 네트워크 복제

HktMass는 Unreal Engine의 MassReplication 플러그인을 활용하여 효율적인 네트워크 복제를 제공합니다.

### 복제 구조
```
Server
  └─ UHktMassNpcReplicator
       ├─ AHktMassNpcClientBubbleInfo (각 클라이언트마다 생성)
       │    └─ FHktReplicatedNpcAgent[] (FastArray)
       └─ FHktMassNpcServerReplicationHelper

Client
  └─ AHktMassNpcClientBubbleInfo (서버에서 복제)
       ├─ FHktReplicatedNpcAgent[] (자동 동기화)
       └─ FHktMassNpcClientReplicationHelper
```

### 복제되는 데이터
- Transform (위치, 회전)
- NPC Type (Melee/Ranged/Tank)
- State (Idle/Patrol/Chase/Attack/Dead)
- Health (현재 체력)
- Animation State (애니메이션 상태 인덱스)

### LOD 기반 복제
- **High LOD** (가까운 거리): 매 프레임 업데이트
- **Medium LOD** (중간 거리): 0.1초마다 업데이트
- **Low LOD** (먼 거리): 1초마다 업데이트

## 🚀 성능 특징

### Mass Entity System
- ✅ 수천 개의 NPC를 동시에 처리
- ✅ 데이터 지향 설계로 캐시 효율 극대화
- ✅ 멀티스레드 처리 지원

### FastArray Serialization
- ✅ 델타 복제로 네트워크 대역폭 절약
- ✅ 변경된 엔티티만 전송
- ✅ 클라이언트별 관심 영역 필터링

### LOD System
- ✅ 거리 기반 업데이트 빈도 조절
- ✅ 네트워크 및 CPU 부하 감소

## 🛠️ 확장 방법

### 새로운 Fragment 추가
1. `Public/Fragments/HktMassNpcFragments.h`에 Fragment 정의
2. `Private/Fragments/HktMassNpcFragments.cpp`에 구현 (필요시)

### 새로운 시스템 추가
1. `Private/YourSystem/` 폴더 생성
2. `UYourSystemTrait.h/cpp` 작성 (Fragment 구성)
3. `UYourSystemProcessor.h/cpp` 작성 (로직 처리)

### 복제 프로퍼티 추가
1. `Public/Types/HktMassNpcReplicationTypes.h`에 필드 추가
2. `Private/Replication/HktMassNpcReplicator.cpp`에서 수집
3. `Private/Replication/HktMassNpcReplicationHelpers.cpp`에서 적용

## 📖 추가 문서

- [STRUCTURE.md](STRUCTURE.md) - 상세한 폴더 구조 설명
- [README_REFACTORED.md](Private/README_REFACTORED.md) - 리팩토링 이력

## 🏆 라이센스

Copyright Hkt Studios, Inc. All Rights Reserved.
