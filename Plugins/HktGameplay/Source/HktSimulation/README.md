# HktSimulation Module

## 개요

HktSimulation은 결정론적 게임 시뮬레이션을 담당하는 **순수 로직** 모듈입니다.
IntentEvent를 입력으로 받아 바이트코드 VM을 통해 실행하며, 엔티티/플레이어 상태를 관리합니다.

**중요**: 이 모듈은 Unreal 네트워킹(Replication)과 직접 연동하지 않습니다.
외부 연동(Component, Replication)은 `HktIntent` 모듈에서 `IHktPlayerAttributeProvider` 인터페이스를 통해 처리합니다.

## 핵심 아키텍처

```
┌─────────────────────────────────────────────────────────────┐
│ HktSimulation (순수 로직)                                   │
│                                                             │
│  IntentEvent → FlowRegistry → FlowBuilder → Bytecode       │
│                                    ↓                        │
│                              VMPool → FlowVM                │
│                                    ↓                        │
│                            OpcodeRegistry → Handlers        │
│                                    ↓                        │
│                    EntityManager ← SpatialIndex             │
│                         ↓      ↓                            │
│               PlayerDatabase  EntityDatabase                │
│                         ↓                                   │
│           IHktPlayerAttributeProvider (인터페이스 노출)     │
└─────────────────────────────────────────────────────────────┘
                            ↓
                  HktService (중개 레이어)
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ HktIntent (Unreal 통합)                                     │
│                                                             │
│  IntentSubsystem → Provider 구독 → AttributeComponent      │
│                                    ↓                        │
│                           FFastArraySerializer              │
│                                    ↓                        │
│                           Replication → Client              │
└─────────────────────────────────────────────────────────────┘
```

## 주요 컴포넌트

### 1. UHktSimulationSubsystem
**책임**: 시뮬레이션 생명주기 관리 + **IHktPlayerAttributeProvider 구현**

- IntentEvent 처리 (Sliding Window)
- VM 생성 및 실행
- Entity/Handle 매핑 관리
- **Player 속성 변경 사항 제공** (Provider 인터페이스)

**사용 예시**:
```cpp
UHktSimulationSubsystem* Sim = UHktSimulationSubsystem::Get(WorldContext);
FUnitHandle Unit = Sim->GetOrCreateInternalHandle(ExternalId, Location);
```

**Provider 사용 (HktIntent에서)**:
```cpp
// HktServiceSubsystem을 통해 Provider 접근
IHktPlayerAttributeProvider* Provider = Service->GetPlayerAttributeProvider().GetInterface();

// 변경된 플레이어 속성 수신
TArray<FHktPlayerAttributeSnapshot> Snapshots;
if (Provider->ConsumeChangedPlayers(Snapshots))
{
    for (const auto& Snapshot : Snapshots)
    {
        // AttributeComponent에 적용
        Component->ApplyAttributeSnapshot(Snapshot);
    }
}
```

### 2. Flow Definition System
**책임**: 이벤트 → 바이트코드 변환 규칙 정의

**Flow 추가하기**:
```cpp
class FMyFlowDefinition : public IFlowDefinition
{
    virtual bool BuildBytecode(FHktFlowBuilder& B, const FHktIntentEvent& E, FHktEntityManager* M) override
    {
        B.PlayAnim(FName("Attack"))
         .WaitSeconds(0.5f)
         .ModifyHealth(1, -50.0f);
        return true;
    }
    
    virtual FGameplayTag GetEventTag() const override
    {
        return TAG_Event_Action_Attack;
    }
};

REGISTER_FLOW_DEFINITION(FMyFlowDefinition)
```

### 3. FHktFlowVM
**책임**: 바이트코드 실행

- 8개 범용 레지스터 (GPR)
- 블로킹/타이머 상태 관리
- Opcode dispatch

**레지스터 사용**:
- GPR[0]: 보통 위치/타겟 저장
- GPR[1]: 보통 타겟 유닛 저장
- GPR[2-7]: 임시 값 저장

### 4. FHktEntityManager
**책임**: 엔티티 및 플레이어 데이터 관리

**주요 API**:
```cpp
// 엔티티 생성/삭제
FUnitHandle Unit = Mgr.AllocUnit(Player, Location, Rotation);
Mgr.FreeUnit(Unit);

// 속성 접근
FHktAttributeSet* Attrs = Mgr.GetUnitAttrs(Unit);
Attrs->Modify(EHktAttribute::Health, -10.0f);

// 공간 쿼리
TArray<FUnitHandle> Results;
Mgr.QueryUnitsInSphere(Center, Radius, Results);
```

### 5. FHktSpatialIndex
**책임**: 효율적인 범위 검색

**성능**:
- O(K) 쿼리 (K = 범위 내 엔티티 수)
- Grid 기반 (기본 셀 크기: 1000 units)
- 자동 업데이트

## 성능 최적화

### Object Pooling

**VM Pool**:
```cpp
// 자동으로 SimulationSubsystem에서 사용
// 수동 제어도 가능:
VMPool->SetMaxPoolSize(200);
VMPool->Prewarm(20);
```

**Bytecode Pool**:
```cpp
FBytecodePool::SetMaxPoolSize(50);
FBytecodePool::Prewarm(10);
```

### Spatial Partitioning

**활성화/비활성화**:
```cpp
EntityManager.SetSpatialIndexEnabled(true);  // 기본값
EntityManager.SetSpatialIndexEnabled(false); // 디버깅용
```

**통계 확인**:
```cpp
int32 Total, Occupied, MaxPerCell;
SpatialIndex->GetStats(Total, Occupied, MaxPerCell);
```

## 성능 모니터링

### Stats 사용법

콘솔에서:
```
stat HktSimulation
```

**주요 메트릭**:
- `STAT_ProcessIntentEvents`: 이벤트 처리 시간
- `STAT_TickVMs`: VM 실행 시간
- `STAT_SpatialQueries`: 공간 쿼리 시간
- `STAT_ActiveVMs`: 활성 VM 수
- `STAT_TotalEntities`: 전체 엔티티 수

### 프로파일링

Unreal Insights와 통합:
```
stat startfile
<play game>
stat stopfile
```

## 테스트

### 단위 테스트 실행

```
Automation RunTests HktSimulation
```

**테스트 항목**:
- EntityManager 기본 작업
- Spatial Index 쿼리
- Flow Registry 조회
- Opcode Registry 작업
- VM Pool 라이프사이클
- Bytecode Pool 메모리 재사용

### 벤치마크 실행

```
Automation RunTests HktSimulation.Benchmarks
```

**벤치마크 항목**:
- Spatial Index vs Linear Search
- VM Pool vs Direct Allocation

## 에러 처리

### Result 타입 사용

```cpp
FSimulationResult result = SomeOperation();
if (result.IsFailure())
{
    UE_LOG(LogSim, Error, TEXT("%s"), *result.ToString());
    return;
}

// 또는 매크로 사용
SIM_RETURN_IF_FAILED(result);
SIM_LOG_IF_FAILED(result, LogSim);
```

### 에러 코드

- `ESimulationError::InvalidHandle`: 유효하지 않은 핸들
- `ESimulationError::FlowDefinitionNotFound`: Flow 정의 없음
- `ESimulationError::BuildFailed`: 바이트코드 빌드 실패
- `ESimulationError::ExecutionFailed`: VM 실행 실패

## 확장 가이드

### 새 Opcode 추가

```cpp
// 1. Opcode 범위 할당
namespace EMyModuleOp
{
    static constexpr HktOpCode Base = 
        FHktOpRegistry::AllocateOpcodeRange(TEXT("MyModule"), 10);
    static constexpr HktOpCode CustomOp = Base + 0;
}

// 2. 핸들러 구현
void Op_CustomOp(FHktFlowVM& VM, uint8* Data)
{
    SCOPE_CYCLE_COUNTER(STAT_Op_CustomOp);
    // 구현
}

// 3. 등록
FHktOpRegistry::RegisterOpcode(EMyModuleOp::CustomOp, Op_CustomOp, TEXT("MyModule"));
```

### 새 Attribute 추가

```cpp
// HktAttributeSet.h에 추가
enum class EHktAttribute : uint8
{
    // 기존...
    MyCustomAttribute,
    Count
};

// 사용
Attrs->Set(EHktAttribute::MyCustomAttribute, 100.0f);
Attrs->Modify(EHktAttribute::MyCustomAttribute, -10.0f);
```

## 디버깅 팁

### VM 실행 추적

```cpp
// 로그 레벨 변경
LogHktSimulation Verbose

// VM 상태 확인
UE_LOG(LogHktSimulation, Verbose, TEXT("VM PC=%d, Blocked=%d"), 
    VM.Regs.ProgramCounter, VM.Regs.bBlocked);
```

### Spatial Index 시각화

```cpp
// 통계 로깅
int32 Total, Occupied, MaxPerCell;
SpatialIndex->GetStats(Total, Occupied, MaxPerCell);
UE_LOG(LogSpatial, Log, TEXT("Total=%d, Cells=%d, Max/Cell=%d"), 
    Total, Occupied, MaxPerCell);
```

### Handle 매핑 추적

```cpp
// External -> Internal 조회
FUnitHandle Internal = GetOrCreateInternalHandle(ExternalId);

// Internal -> External 조회
int32 External = GetExternalUnitId(InternalHandle);
```

## 모범 사례

### Flow Definition
- **단일 책임**: 각 Flow는 하나의 이벤트 타입만 처리
- **상태 비저장**: Flow는 stateless여야 함
- **빠른 빌드**: BuildBytecode는 가볍게 유지

### VM 사용
- **레지스터 관리**: 일관된 레지스터 사용 규칙 유지
- **블로킹 최소화**: 너무 긴 Wait는 피할 것
- **조기 종료**: 불필요한 바이트코드 실행 방지

### Entity 관리
- **핸들 검증**: 항상 IsUnitValid() 확인
- **속성 접근**: null 체크 후 사용
- **공간 쿼리**: 적절한 반경 사용 (너무 크지 않게)

## 빌드 정보

**Dependencies**:
- MassEntity
- MassCommon
- GameplayTags
- StructUtils

**Compiler**: C++17 권장 (C++14 최소)

**Platform**: Win64, Linux (테스트됨)

## 문제 해결

### Q: VM이 실행되지 않음
A: ActiveVMs 배열 확인, 바이트코드가 비어있지 않은지 확인

### Q: Spatial Query가 느림
A: 셀 크기 조정, 엔티티 수 확인, stat HktSimulation으로 프로파일링

### Q: 메모리 사용량이 높음
A: Pool 크기 조정, 엔티티 정리 확인, 공간 인덱스 비활성화 시도

### Q: Flow가 등록되지 않음
A: REGISTER_FLOW_DEFINITION 매크로 사용 확인, 모듈 로드 순서 확인

## 기여 가이드

1. 새 기능 추가 시 테스트 작성
2. 성능 영향 벤치마크
3. Stats 메트릭 추가
4. 문서 업데이트

## 라이선스

Copyright Hkt Studios, Inc. All Rights Reserved.

---

**For more details, see**: `REFACTORING_SUMMARY.md`
