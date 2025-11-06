# HktProto Mass Entity 시스템 - 빠른 시작 가이드

## ✅ 구현 완료 항목

Mass Entity 시스템을 사용한 NPC 생성 및 이동 패턴 구현이 완료되었습니다!

### 생성된 파일들

```
Source/HktProto/MassEntity/
├── HktMassNpcFragments.h/cpp           - NPC 데이터 구조 (Fragment)
├── HktMassNpcMovementProcessor.h/cpp   - 이동 처리 시스템
├── HktMassNpcAIProcessor.h/cpp         - AI 로직 (Idle, Patrol, Chase, Attack)
├── HktMassNpcVisualizationProcessor.h/cpp - 렌더링 처리
├── HktMassNpcTrait.h/cpp               - 엔티티 구성 (Melee, Ranged, Tank)
├── HktMassNpcSpawner.h/cpp             - NPC 생성 관리 액터
├── HktMassEntityModule.h/cpp           - 모듈 초기화
└── README.md                           - 상세 문서
```

### 수정된 파일들

- `HktProto.Build.cs` - Mass Entity 모듈 의존성 추가
- `HktProto.uproject` - Mass Entity 플러그인 활성화

## 🚀 사용 방법

### 1단계: 프로젝트 빌드

1. **Visual Studio에서 빌드**
   - `HktProto.sln` 파일을 Visual Studio로 엽니다
   - 솔루션 빌드 (Ctrl+Shift+B)

2. **또는 언리얼 에디터에서 빌드**
   - `HktProto.uproject` 파일을 더블클릭하여 에디터 실행
   - 자동으로 컴파일됩니다

### 2단계: Mass Entity Config Asset 생성

1. **Content Browser**에서 우클릭
2. **Mass** > **Mass Entity Config Asset** 생성
3. 다음 3개의 Config 생성:
   - `DA_MeleeNpc`
   - `DA_RangedNpc`
   - `DA_TankNpc`

### 3단계: Config Asset 설정

각 Config Asset을 열고 **Traits** 섹션에 추가:

#### DA_MeleeNpc
- **Traits** 배열에 추가
  - **Hkt Melee Npc Trait**
    - Max Speed: 350
    - Max Health: 150
    - Attack Power: 20
    - Attack Range: 100

#### DA_RangedNpc
- **Traits** 배열에 추가
  - **Hkt Ranged Npc Trait**
    - Max Speed: 300
    - Max Health: 80
    - Attack Power: 15
    - Attack Range: 500

#### DA_TankNpc
- **Traits** 배열에 추가
  - **Hkt Tank Npc Trait**
    - Max Speed: 250
    - Max Health: 300
    - Attack Power: 10
    - Attack Range: 100

### 4단계: 레벨에 Spawner 배치

1. **Place Actors** 패널에서 "HktMassNpcSpawner" 검색
2. 레벨에 드래그 앤 드롭
3. Details 패널에서 설정:

```yaml
Components:
  - Melee Npc Mesh ISM:
      Static Mesh: /Game/StarterContent/Shapes/Shape_Cone
  
  - Ranged Npc Mesh ISM:
      Static Mesh: /Game/StarterContent/Shapes/Shape_Cylinder
  
  - Tank Npc Mesh ISM:
      Static Mesh: /Game/StarterContent/Shapes/Shape_Cube

Spawn:
  - Spawn Configs:
      [0]:
        Entity Config: DA_MeleeNpc
        Spawn Count: 30
        Spawn Radius: 1000.0
        Min Spacing: 100.0
      
      [1]:
        Entity Config: DA_RangedNpc
        Spawn Count: 20
        Spawn Radius: 1500.0
        Min Spacing: 150.0
      
      [2]:
        Entity Config: DA_TankNpc
        Spawn Count: 10
        Spawn Radius: 800.0
        Min Spacing: 200.0
  
  - Auto Spawn On Begin Play: ✓ true
  - Spawn Delay: 0.0

Debug:
  - Draw Debug Spawn Area: ✓ true
```

### 5단계: 플레이!

**Play** 버튼을 누르면 NPC들이 자동으로 생성되고 다음과 같이 동작합니다:

- ✅ **자동 순찰** - 랜덤한 경로를 따라 이동
- ✅ **자연스러운 이동** - 부드러운 회전과 이동
- ✅ **AI 상태 전환** - Idle → Patrol → Chase → Attack
- ✅ **효율적인 렌더링** - Instanced Static Mesh 사용

## 🎮 블루프린트에서 제어

### NPC 동적 생성

```
Get Actor Of Class (AHktMassNpcSpawner)
  → SpawnNpcs()
```

### NPC 전체 제거

```
Get Actor Of Class (AHktMassNpcSpawner)
  → DespawnAllNpcs()
```

### 특정 설정으로 생성

```
Get Actor Of Class (AHktMassNpcSpawner)
  → Make HktNpcSpawnConfig
      Entity Config: DA_MeleeNpc
      Spawn Count: 100
      Spawn Radius: 2000.0
      Min Spacing: 100.0
  → SpawnNpcsWithConfig
```

## 🔍 디버깅 팁

### Mass Entity Debugger
- **Window** > **Mass** > **Entity Debugger**
- 실시간으로 엔티티 상태 확인

### 콘솔 명령어
```
mass.debug 1              // Mass 디버그 정보 표시
mass.debug.shapes 1       // 디버그 셰이프 표시
showdebug mass            // Mass 통계 표시
```

### 로그 확인
```
LogTemp: Spawned NPCs from X configs
LogTemp: Spawned X NPCs at location: ...
```

## 📊 성능 확인

### 통계 표시
에디터에서 **` (백틱)** 키를 눌러 콘솔을 열고:
```
stat fps          // FPS 표시
stat unit         // 프레임 시간 표시
stat mass         // Mass Entity 통계
```

### 예상 성능
- **100 NPCs**: 매우 부드러움 (60+ FPS)
- **500 NPCs**: 부드러움 (45+ FPS)
- **1000+ NPCs**: 최적화 필요 (LOD 시스템 권장)

## ⚠️ API 업데이트 사항

UE5의 Mass Entity API가 업데이트되어 다음 사항들이 변경되었습니다:

### ConfigureQueries 시그니처 변경
```cpp
// 이전
virtual void ConfigureQueries() override;

// 현재 (UE5.5+)
virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
```

### ForEachEntityChunk 파라미터 변경
```cpp
// 이전
EntityQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& Context) {});

// 현재 (UE5.5+)
EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context) {});
```

### ProcessorGroupNames 변경
```cpp
// 이전
ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;

// 현재 (UE5.5+)
ExecutionOrder.ExecuteInGroup = FName(TEXT("Movement"));
```

### GetOrCreateEntityTemplate 변경
```cpp
// 이전
const FMassEntityTemplate* Template = Config.GetOrCreateEntityTemplate(*this, *Config);

// 현재 (UE5.5+)
const FMassEntityTemplate& Template = Config.GetOrCreateEntityTemplate(*GetWorld());
```

## 🛠️ 커스터마이징

### 새로운 NPC 타입 추가

1. **HktMassNpcTrait.h**에 새 클래스 추가:
```cpp
UCLASS(meta = (DisplayName = "Hkt Boss Npc Trait"))
class UHktMassBossNpcTrait : public UHktMassNpcTrait
{
    GENERATED_BODY()

public:
    UHktMassBossNpcTrait()
    {
        NpcType = 3;
        MaxSpeed = 400.0f;
        MaxHealth = 1000.0f;
        AttackPower = 50.0f;
    }
};
```

2. Config Asset 생성 및 Trait 추가
3. Spawner에 ISM 컴포넌트 추가
4. Visualization Processor에 case 추가

### 순찰 경로 지정

Spawner에서 순찰 경로를 설정하려면:

```cpp
// C++ 코드
FHktNpcPatrolFragment& PatrolFragment = ...;
PatrolFragment.PatrolPoints.Add(FVector(1000, 0, 0));
PatrolFragment.PatrolPoints.Add(FVector(1000, 1000, 0));
PatrolFragment.PatrolPoints.Add(FVector(0, 1000, 0));
PatrolFragment.PatrolPoints.Add(FVector(0, 0, 0));
```

## 🎯 보스 액터 통합

일부 보스는 별도의 Actor로 구현하고 싶다면:

1. **AActor** 기반 보스 클래스 생성
2. Mass Entity NPC와 상호작용하도록 타겟 시스템 통합
3. **FHktNpcTargetFragment**에서 보스 액터를 타겟으로 지정

```cpp
// 보스 액터를 타겟으로 설정
TargetFragment.TargetLocation = BossActor->GetActorLocation();
TargetFragment.bHasValidTarget = true;
```

## 🌐 네트워크 리플리케이션 (향후)

현재는 로컬 전용입니다. 멀티플레이어 지원을 위해:

1. **MassReplication** 모듈 통합
2. **FMassReplicatedAgentFragment** 추가
3. 중요한 NPC만 리플리케이트 (보스, 특수 유닛)
4. 일반 NPC는 각 클라이언트에서 로컬로 생성

## 📚 추가 학습 자료

- **Source/HktProto/MassEntity/README.md** - 상세 문서
- [UE5 Mass Entity 공식 문서](https://docs.unrealengine.com/5.0/en-US/overview-of-mass-entity-in-unreal-engine/)
- [City Sample Project](https://docs.unrealengine.com/5.0/en-US/city-sample-project-unreal-engine-demonstration/)

## 🐛 문제 해결

### "MassEntitySubsystem is null" 에러
- Mass Entity 플러그인이 활성화되지 않았습니다
- .uproject 파일에서 플러그인 확인

### NPC가 보이지 않음
- Spawner의 ISM에 Static Mesh가 할당되었는지 확인
- Config Asset에 Trait가 추가되었는지 확인

### NPC가 움직이지 않음
- Processor가 등록되었는지 확인
- AI 상태가 올바른지 디버거로 확인

### 컴파일 에러
- Visual Studio에서 전체 리빌드
- Intermediate, Binaries 폴더 삭제 후 재빌드

## ✨ 완성!

이제 Mass Entity를 사용한 대규모 NPC 시스템이 준비되었습니다!

궁금한 점이 있으면 `Source/HktProto/MassEntity/README.md`를 참고하세요.

---

**HktProto RTS/RPG Defense Game**  
Powered by Unreal Engine 5 Mass Entity System

