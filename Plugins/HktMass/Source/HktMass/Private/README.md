# HktProto Mass Entity System

언리얼 엔진 5의 Mass Entity 시스템을 사용하여 RTS/RPG 디펜스 게임의 NPC를 구현한 시스템입니다.

## 📋 개요

이 시스템은 대량의 NPC를 효율적으로 관리하고 렌더링하기 위해 Mass Entity를 활용합니다.
- **대규모 엔티티 관리**: 수천 개의 NPC를 동시에 처리 가능
- **ECS 아키텍처**: Entity-Component-System 패턴 사용
- **최적화된 렌더링**: Instanced Static Mesh를 통한 효율적인 렌더링

## 🏗️ 구조

### 1. Fragments (데이터 구조)
`HktMassNpcFragments.h/cpp`에 정의된 Fragment들:

- **FHktNpcMovementFragment**: 이동 속성 (속도, 방향)
- **FHktNpcCombatFragment**: 전투 속성 (체력, 공격력, 공격 범위)
- **FHktNpcTargetFragment**: 타겟 정보
- **FHktNpcStateFragment**: AI 상태 (Idle, Patrol, Chase, Attack, Dead)
- **FHktNpcTypeFragment**: NPC 타입 정보
- **FHktNpcPatrolFragment**: 순찰 경로 정보
- **FHktNpcVisualizationFragment**: 시각화 정보

### 2. Processors (로직/시스템)

#### UHktMassNpcMovementProcessor
NPC의 이동을 처리합니다.
- 타겟 방향으로 이동
- 회전 업데이트
- Dead 상태 체크

#### UHktMassNpcAIProcessor
NPC의 AI 로직을 처리합니다.
- **Idle**: 대기 상태
- **Patrol**: 순찰 (랜덤 또는 지정된 경로)
- **Chase**: 타겟 추적
- **Attack**: 공격
- **Dead**: 사망

#### UHktMassNpcPatrolProcessor
순찰 경로를 관리합니다.
- 순찰 포인트 자동 생성

#### UHktMassNpcVisualizationProcessor
NPC의 시각적 표현을 관리합니다.
- Instanced Static Mesh 업데이트
- Transform 동기화

### 3. Traits (엔티티 구성)
`HktMassNpcTrait.h/cpp`에 정의된 Trait들:

- **UHktMassNpcTrait**: 기본 NPC Trait
- **UHktMassMeleeNpcTrait**: 근접 공격 NPC
  - 최대 속도: 350
  - 체력: 150
  - 공격력: 20
  - 공격 범위: 100
  
- **UHktMassRangedNpcTrait**: 원거리 공격 NPC
  - 최대 속도: 300
  - 체력: 80
  - 공격력: 15
  - 공격 범위: 500
  
- **UHktMassTankNpcTrait**: 탱커 NPC
  - 최대 속도: 250
  - 체력: 300
  - 공격력: 10
  - 공격 범위: 100

### 4. Spawner (생성 관리)
`HktMassNpcSpawner.h/cpp`

**AHktMassNpcSpawner**: 맵에 배치하여 NPC를 생성하는 액터

## 🚀 사용 방법

### 1. 프로젝트 설정

#### A. 플러그인 활성화
`HktProto.uproject` 파일에 이미 활성화되어 있습니다:
- MassEntity
- MassGameplay
- MassAI

#### B. 모듈 의존성
`HktProto.Build.cs`에 이미 추가되어 있습니다:
```csharp
"MassEntity",
"MassCommon",
"MassMovement",
"MassSpawner",
"MassActors",
"MassRepresentation",
"MassLOD",
"MassSimulation",
"StructUtils",
"ZoneGraph"
```

### 2. 블루프린트에서 Entity Config 생성

1. **Content Browser**에서 우클릭
2. **Mass** > **Mass Entity Config Asset** 생성
3. 이름: `DA_MeleeNpc`, `DA_RangedNpc`, `DA_TankNpc` 등

4. Config Asset을 열고 **Traits** 추가:
   - `Hkt Melee Npc Trait` (근접 NPC용)
   - `Hkt Ranged Npc Trait` (원거리 NPC용)
   - `Hkt Tank Npc Trait` (탱커 NPC용)

### 3. Spawner 배치

1. 레벨에 **AHktMassNpcSpawner** 액터 배치
2. Details 패널에서 설정:

```
Spawn Configs:
  [0]:
    Entity Config: DA_MeleeNpc
    Spawn Count: 50
    Spawn Radius: 1000.0
    Min Spacing: 100.0
    
  [1]:
    Entity Config: DA_RangedNpc
    Spawn Count: 30
    Spawn Radius: 1500.0
    Min Spacing: 150.0
```

3. **Auto Spawn On Begin Play**: true
4. **Spawn Delay**: 0.0 (즉시 스폰)

### 4. Static Mesh 설정

Spawner의 ISM 컴포넌트에 메시 할당:
- **Melee Npc Mesh ISM**: 근접 NPC용 메시
- **Ranged Npc Mesh ISM**: 원거리 NPC용 메시
- **Tank Npc Mesh ISM**: 탱커 NPC용 메시

예: `/Game/StarterContent/Shapes/Shape_Cone`

### 5. 블루프린트에서 제어

```cpp
// C++ 코드
AHktMassNpcSpawner* Spawner = ...; // 스포너 참조

// NPC 생성
Spawner->SpawnNpcs();

// 모든 NPC 제거
Spawner->DespawnAllNpcs();

// 특정 설정으로 생성
FHktNpcSpawnConfig Config;
Config.SpawnCount = 100;
Config.SpawnRadius = 2000.0f;
Spawner->SpawnNpcsWithConfig(Config);
```

블루프린트에서도 동일한 함수들을 호출할 수 있습니다.

## 🎮 AI 상태 시스템

NPC는 다음과 같은 상태를 가집니다:

### 0: Idle (대기)
- 2초 후 자동으로 Patrol 상태로 전환

### 1: Patrol (순찰)
- 순찰 포인트가 설정되어 있으면 해당 경로를 따라 이동
- 순찰 포인트가 없으면 랜덤한 위치로 이동 (5초마다)
- 각 포인트에서 대기 시간 적용

### 2: Chase (추적)
- 타겟을 추적하여 이동
- 5초 후 Patrol 상태로 전환 (현재 단순화된 구현)

### 3: Attack (공격)
- 공격 범위 내에서 타겟 공격
- 공격 쿨다운 적용
- 타겟이 범위 밖으로 나가면 Chase 상태로 전환

### 4: Dead (사망)
- 체력이 0 이하일 때
- 더 이상 업데이트되지 않음

## 🔧 커스터마이징

### 새로운 NPC 타입 추가

1. **새로운 Trait 클래스 생성**
```cpp
UCLASS(meta = (DisplayName = "Hkt Boss Npc Trait"))
class UHktMassBossNpcTrait : public UHktMassNpcTrait
{
    GENERATED_BODY()

public:
    UHktMassBossNpcTrait()
    {
        NpcType = 3; // Boss
        MaxSpeed = 400.0f;
        MaxHealth = 1000.0f;
        AttackPower = 50.0f;
        AttackRange = 300.0f;
        AttackCooldown = 3.0f;
    }
};
```

2. **Spawner에 새로운 ISM 추가**
```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
TObjectPtr<UInstancedStaticMeshComponent> BossNpcMeshISM;
```

3. **Visualization Processor 업데이트**
```cpp
case 3: // Boss
    TargetISM = BossNpcMeshISM;
    break;
```

### Fragment 추가

새로운 데이터가 필요하면 Fragment를 추가하세요:

```cpp
USTRUCT()
struct FHktNpcSpecialAbilityFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY()
    float AbilityCooldown = 10.0f;
    
    UPROPERTY()
    float LastAbilityTime = 0.0f;
};
```

그리고 Trait의 `BuildTemplate`에서 추가:
```cpp
BuildContext.AddFragment<FHktNpcSpecialAbilityFragment>();
```

### Processor 추가

새로운 로직이 필요하면 Processor를 만드세요:

```cpp
UCLASS()
class UHktMassNpcSpecialAbilityProcessor : public UMassProcessor
{
    GENERATED_BODY()
    
    // ... 구현
};
```

## 📊 성능 최적화 팁

1. **LOD 시스템 활용**: MassLOD를 사용하여 거리에 따라 업데이트 빈도 조절
2. **쿼리 최적화**: Fragment 접근을 ReadOnly로 설정하여 성능 향상
3. **Chunk 단위 처리**: ForEachEntityChunk를 사용하여 캐시 효율성 향상
4. **ISM 사용**: 많은 NPC를 효율적으로 렌더링

## 🐛 디버깅

### 디버그 드로잉
Spawner의 `bDrawDebugSpawnArea`를 true로 설정하면 스폰 영역이 표시됩니다.

### 로그
```cpp
UE_LOG(LogTemp, Log, TEXT("NPC State: %d"), StateFragment.CurrentState);
```

### Mass Entity 디버거
에디터에서 **Window** > **Mass** > **Entity Debugger**를 열어 엔티티 상태를 실시간으로 확인할 수 있습니다.

## 📚 참고 자료

- [Unreal Engine Mass Entity Documentation](https://docs.unrealengine.com/5.0/en-US/overview-of-mass-entity-in-unreal-engine/)
- [City Sample Project](https://docs.unrealengine.com/5.0/en-US/city-sample-project-unreal-engine-demonstration/)

## 🔄 향후 개선 사항

- [ ] 네트워크 리플리케이션 지원
- [ ] 플레이어 감지 및 타겟팅 시스템
- [ ] 더 복잡한 AI 패턴 (State Tree 통합)
- [ ] 스킬 시스템 통합
- [ ] Formation 시스템 (부대 대형)
- [ ] 동적 장애물 회피
- [ ] 보스 액터와의 통합

## 📝 라이선스

Copyright Epic Games, Inc. All Rights Reserved.

