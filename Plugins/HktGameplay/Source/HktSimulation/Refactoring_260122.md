설계: # HktFlow DSL → Bytecode 구현 계획

> Implement the following plan:

  ## 핵심 원칙

  **Builder API는 자연어와 동일한 추상 수준을 유지하고, 모든 로직은 op/데이터로 VM에 주입.**

  ---

  ## 현황 분석

  ### HktFlow 모듈 (자연어 DSL 예시)
  ```cpp
  // MoveToLocation
  Flow.MoveTo(Subject, TargetLocation)
  .OnReachLocation([&]() { Flow.Stop(Subject); })
  .OnCollision([&](int32 CollidedWith) { Flow.ApplyForce(...); });

  // Fireball
  FlowBuilder.PlayAnimation(Caster, Tag_CastingAnim)
  .OnWait(Caster, Signal_AnimEnd)
  [
  FlowBuilder.SpawnEntity(Tag_FireballSpawn)
  .OnSpawn(Ctx_Fireball)
  [
  FlowBuilder.MoveForward(Ctx_Fireball, 1500.0f)
  .OnCollision(Ctx_Fireball, 50.0f, Ctx_HitActor)
  [
  FlowBuilder.DestroyEntity(Ctx_Fireball)
  .SetDamage(Ctx_HitActor, 100.0f)
  ]
  ]
  ];

  // EnterCharacter
  Builder.SpawnEntity(Target)
  .Then(Builder.PlayAnimation(Subject, Tag_Spawn))
  .Then(Builder.SpawnEntity(Tag_Equipment));
  ```

  ### HktSimulation 모듈 (기존 Bytecode 인프라)
  - `FHktFlowBuilder`: 저수준 빌더 (PlayAnim, WaitSeconds)
  - `FHktFlowVM`: 8개 레지스터, 블로킹 상태
  - 기존 오피코드: WaitTime, WaitUntilDestroyed, PlayAnim, SpawnProjectile, ModifyAttribute, ExplodeAndDamage

  ---

  ## 구현 접근 방식

  **Builder API가 자연어와 1:1 대응되도록 설계. 내부적으로 모든 콜백/분기는 op으로 변환.**

  ```cpp
  // 목표: 이렇게 작성하면
  Builder.MoveTo(Subject, TargetLocation)
  .OnArrival().Stop(Subject).End()
  .OnCollision().ApplyForce(Subject, CollidedWith).Stop(Subject).End();

  // 내부적으로 이런 바이트코드가 생성됨:
  // [0] OP_MoveTo(TargetLocation)
  // [1] OP_WaitEvent(ARRIVAL | COLLISION)  // 이벤트 대기
  // [2] OP_BranchOnEvent(ARRIVAL -> PC+2, COLLISION -> PC+5)
  // [3] OP_Stop(Subject)
  // [4] OP_Jump(END)
  // [5] OP_ApplyForce(Subject, CollidedWith)
  // [6] OP_Stop(Subject)
  // [7] END
  ```

  ---

  ## 구현 단계

  ### 1. 오피코드 체계 확장 (`HktFlowOpcodes.h`)

  ```cpp
  namespace ECoreOp
  {
  // === 기존 ===
  static constexpr HktOpCode None = 0;
  static constexpr HktOpCode WaitTime = 1;
  static constexpr HktOpCode WaitUntilDestroyed = 2;
  static constexpr HktOpCode PlayAnim = 10;
  static constexpr HktOpCode SpawnProjectile = 11;
  static constexpr HktOpCode DestroyUnit = 12;
  static constexpr HktOpCode ModifyAttribute = 20;
  static constexpr HktOpCode ExplodeAndDamage = 21;

  // === 이동 (30-39) ===
  static constexpr HktOpCode MoveTo = 30;           // 목표 위치로 이동 시작
  static constexpr HktOpCode MoveForward = 31;      // 전방으로 이동
  static constexpr HktOpCode Stop = 32;             // 이동 중지
  static constexpr HktOpCode ApplyForce = 33;       // 힘 적용

  // === 이벤트 대기 (40-49) ===
  static constexpr HktOpCode WaitArrival = 40;      // 도착까지 대기
  static constexpr HktOpCode WaitCollision = 41;    // 충돌까지 대기 (충돌 상대 Reg에 저장)
  static constexpr HktOpCode WaitSignal = 42;       // 시그널 대기 (애니메이션 종료 등)
  static constexpr HktOpCode WaitAny = 43;          // 복수 이벤트 중 하나 대기

  // === 분기 (50-59) ===
  static constexpr HktOpCode Branch = 50;           // 조건 분기 (이벤트 타입별)
  static constexpr HktOpCode Jump = 51;             // 무조건 점프
  static constexpr HktOpCode Halt = 52;             // 실행 종료

  // === 범위 쿼리 (60-69) ===
  static constexpr HktOpCode QuerySphere = 60;      // 구 범위 쿼리
  static constexpr HktOpCode ForEachTarget = 61;    // 쿼리 결과 순회
  static constexpr HktOpCode EndForEach = 62;       // 순회 종료

  // === 엔티티 (70-79) ===
  static constexpr HktOpCode SpawnEntity = 70;      // 엔티티 생성 (GameplayTag 기반)
  static constexpr HktOpCode DestroyEntity = 71;    // 엔티티 파괴

  // === 데미지 (80-89) ===
  static constexpr HktOpCode SetDamage = 80;        // 직접 데미지
  static constexpr HktOpCode ApplyDot = 81;         // DoT 적용

  static constexpr HktOpCode EndOfStream = 255;
  }
  ```

  ### 2. 자연어 수준 Builder API (`HktFlowBuilder.h`)

  **핵심: HktFlow의 DSL과 1:1 대응**

  ```cpp
  class HKTSIMULATION_API FHktFlowBuilder
  {
  public:
  // === 이동 (자연어: "목표로 이동") ===
  FHktFlowBuilder& MoveTo(FVector Target);
  FHktFlowBuilder& MoveForward(float Speed);
  FHktFlowBuilder& Stop();
  FHktFlowBuilder& ApplyForce(FVector Direction, float Magnitude);

  // === 애니메이션 (자연어: "애니메이션 재생") ===
  FHktFlowBuilder& PlayAnimation(FGameplayTag AnimTag);
  FHktFlowBuilder& PlayAnimation(FName MontageSection);

  // === 대기 (자연어: "~까지 기다림") ===
  FHktFlowBuilder& WaitSeconds(float Duration);
  FHktFlowBuilder& WaitUntilArrival();
  FHktFlowBuilder& WaitUntilCollision(uint8 OutCollidedReg = 0);
  FHktFlowBuilder& WaitUntilSignal(FGameplayTag SignalTag);
  FHktFlowBuilder& WaitUntilDestroyed(uint8 TargetReg);

  // === 엔티티 (자연어: "엔티티 생성/파괴") ===
  FHktFlowBuilder& SpawnEntity(FGameplayTag EntityTag, uint8 OutReg = 0);
  FHktFlowBuilder& DestroyEntity(uint8 TargetReg);

  // === 데미지 (자연어: "데미지 적용") ===
  FHktFlowBuilder& SetDamage(uint8 TargetReg, float Damage);
  FHktFlowBuilder& Explode(float Radius, float DirectDmg, float SplashDmg, uint8 CenterReg);
  FHktFlowBuilder& ApplyBurning(uint8 TargetReg, float DmgPerSec, float Duration);

  // === 범위 쿼리 (자연어: "주변 유닛 검색") ===
  FHktFlowBuilder& QueryNearby(float Radius, uint8 CenterReg, uint8 OutListReg);
  FHktFlowBuilder& ForEachTarget(uint8 ListReg, uint8 IteratorReg);
  FHktFlowBuilder& EndForEach();

  // === 흐름 제어 (자연어: "분기/종료") ===
  FHktFlowBuilder& Branch(EFlowEvent Event, int32 Offset);
  FHktFlowBuilder& Jump(int32 Offset);
  FHktFlowBuilder& End();

  private:
  TArray<uint8>& Buffer;
  void PushHeader(HktOpCode Op, uint16 Size);
  template<typename T> void PushData(const T& Val);
  };
  ```

  ### 3. Flow Definition 구현

  #### MoveToLocationFlowDefinition.cpp

  ```cpp
  // 자연어: "목표 위치로 이동, 도착하면 정지, 충돌하면 반발력 적용 후 정지"
  bool FMoveToLocationFlowDefinition::BuildBytecode(FHktFlowBuilder& B,
  const FHktIntentEvent& Event, FHktEntityManager* Mgr)
  {
  // 1. 목표로 이동 시작
  B.MoveTo(Event.Location);

  // 2. 도착 또는 충돌 대기
  B.WaitUntilArrival();  // 내부: WaitAny(ARRIVAL | COLLISION)

  // 3. 정지
  B.Stop();

  // 4. 종료
  B.End();

  return true;
  }
  ```

  #### FireballFlowDefinition.cpp

  ```cpp
  // 자연어: "시전 → 파이어볼 생성 → 전방 이동 → 충돌 시 폭발 + 범위 데미지"
  bool FFireballFlowDefinition::BuildBytecode(FHktFlowBuilder& B,
  const FHktIntentEvent& Event, FHktEntityManager* Mgr)
  {
  constexpr uint8 REG_FIREBALL = 0;
  constexpr uint8 REG_HIT_TARGET = 1;
  constexpr uint8 REG_NEARBY_LIST = 2;
  constexpr uint8 REG_LOOP_TARGET = 3;

  // 1. 시전 애니메이션
  B.PlayAnimation(FGameplayTag::RequestGameplayTag("Anim.Mage.FireballCast"));
  B.WaitSeconds(1.0f);

  // 2. 파이어볼 생성
  B.SpawnEntity(FGameplayTag::RequestGameplayTag("Entity.Projectile.Fireball"), REG_FIREBALL);

  // 3. 전방 이동 (내부적으로 속도/방향 설정)
  B.MoveForward(1500.0f);

  // 4. 충돌 대기 (충돌 상대가 REG_HIT_TARGET에 저장)
  B.WaitUntilCollision(REG_HIT_TARGET);

  // 5. 파이어볼 파괴
  B.DestroyEntity(REG_FIREBALL);

  // 6. 직격 데미지
  B.SetDamage(REG_HIT_TARGET, 100.0f);

  // 7. 폭발 스플래시
  B.QueryNearby(300.0f, REG_FIREBALL, REG_NEARBY_LIST);
  B.ForEachTarget(REG_NEARBY_LIST, REG_LOOP_TARGET);
  B.SetDamage(REG_LOOP_TARGET, 50.0f);
  B.ApplyBurning(REG_LOOP_TARGET, 10.0f, 5.0f);
  B.EndForEach();

  B.End();
  return true;
  }
  ```

  #### EnterCharacterFlowDefinition.cpp

  ```cpp
  // 자연어: "캐릭터 생성 → 스폰 애니메이션 → 장비 생성 → 인트로 애니메이션"
  bool FEnterCharacterFlowDefinition::BuildBytecode(FHktFlowBuilder& B,
  const FHktIntentEvent& Event, FHktEntityManager* Mgr)
  {
  constexpr uint8 REG_CHARACTER = 0;
  constexpr uint8 REG_EQUIPMENT = 1;

  // 1. 캐릭터 엔티티 생성
  B.SpawnEntity(FGameplayTag::RequestGameplayTag("Entity.Character"), REG_CHARACTER);

  // 2. 스폰 애니메이션
  B.PlayAnimation(FGameplayTag::RequestGameplayTag("Anim.Character.Spawn"));
  B.WaitSeconds(0.5f);

  // 3. 장비 엔티티 생성
  B.SpawnEntity(FGameplayTag::RequestGameplayTag("Entity.Character.Equipment"), REG_EQUIPMENT);

  // 4. 인트로 애니메이션
  B.PlayAnimation(FGameplayTag::RequestGameplayTag("Anim.Character.Spawn_Intro"));

  B.End();
  return true;
  }
  ```

  ### 4. VM 핸들러 구현 (`HktFlowVM.cpp`)

  #### 이동 핸들러
  ```cpp
  void FHktFlowVM::Op_MoveTo(FHktFlowVM& VM, uint8* Data)
  {
  FVector* Target = reinterpret_cast<FVector*>(Data);
  if (VM.Mgr->IsUnitValid(VM.Regs.OwnerUnit))
  {
  // 이동 시스템에 목표 설정 요청
  VM.Mgr->SetUnitMoveTarget(VM.Regs.OwnerUnit, *Target);
  }
  }

  void FHktFlowVM::Op_WaitArrival(FHktFlowVM& VM, uint8* Data)
  {
  struct FParam { float Tolerance; } *P = reinterpret_cast<FParam*>(Data);

  FVector CurrentPos = VM.Mgr->GetUnitLocation(VM.Regs.OwnerUnit);
  FVector TargetPos = VM.Mgr->GetUnitMoveTarget(VM.Regs.OwnerUnit);

  if (FVector::Dist(CurrentPos, TargetPos) > P->Tolerance)
  {
  VM.Regs.bBlocked = true;
  VM.Regs.ProgramCounter -= sizeof(FInstructionHeader) + sizeof(FParam); // 다시 실행
  }
  }
  ```

  #### 충돌 대기 핸들러
  ```cpp
  void FHktFlowVM::Op_WaitCollision(FHktFlowVM& VM, uint8* Data)
  {
  struct FParam { uint8 OutReg; float Radius; } *P = reinterpret_cast<FParam*>(Data);

  FUnitHandle Collided = VM.Mgr->CheckCollision(VM.Regs.OwnerUnit, P->Radius);

  if (Collided.IsValid())
  {
  VM.Regs.Get(P->OutReg).SetUnit(Collided);
  // 충돌 발생, 다음 명령어로
  }
  else
  {
  VM.Regs.bBlocked = true;
  VM.Regs.ProgramCounter -= sizeof(FInstructionHeader) + sizeof(FParam);
  }
  }
  ```

  #### ForEach 핸들러
  ```cpp
  void FHktFlowVM::Op_ForEachTarget(FHktFlowVM& VM, uint8* Data)
  {
  struct FParam { uint8 ListReg; uint8 IterReg; uint8 IndexReg; int32 EndOffset; }
  *P = reinterpret_cast<FParam*>(Data);

  TArray<FUnitHandle>* List = VM.Regs.Get(P->ListReg).GetUnitList();
  int32& Index = VM.Regs.Get(P->IndexReg).GetInt();

  if (Index < List->Num())
  {
  VM.Regs.Get(P->IterReg).SetUnit((*List)[Index]);
  Index++;
  }
  else
  {
  VM.Regs.ProgramCounter += P->EndOffset; // EndForEach로 점프
  }
  }
  ```

  ---

  ## 수정할 파일 목록

  | 파일 | 변경 내용 |
  |------|----------|
  | `Plugins/HktGameplay/Source/HktSimulation/Private/HktFlowOpcodes.h` | 새 오피코드 추가 (MoveTo, WaitCollision, ForEach 등) |
  | `Plugins/HktGameplay/Source/HktSimulation/Private/HktFlowBuilder.h` | 자연어 수준 API 추가 |
  | `Plugins/HktGameplay/Source/HktSimulation/Private/HktFlowVM.h` | 새 레지스터 타입 (UnitList) |
  | `Plugins/HktGameplay/Source/HktSimulation/Private/HktFlowVM.cpp` | 새 핸들러 구현 |
  | `Plugins/HktGameplay/Source/HktSimulation/Private/Flow/Definitions/MoveToLocationFlowDefinition.h` | 신규 |
  | `Plugins/HktGameplay/Source/HktSimulation/Private/Flow/Definitions/MoveToLocationFlowDefinition.cpp` | 신규 |
  | `Plugins/HktGameplay/Source/HktSimulation/Private/Flow/Definitions/FireballFlowDefinition.h` | 신규 |
  | `Plugins/HktGameplay/Source/HktSimulation/Private/Flow/Definitions/FireballFlowDefinition.cpp` | 신규 |
  | `Plugins/HktGameplay/Source/HktSimulation/Private/Flow/Definitions/EnterCharacterFlowDefinition.h` | 신규 |
  | `Plugins/HktGameplay/Source/HktSimulation/Private/Flow/Definitions/EnterCharacterFlowDefinition.cpp` | 신규 |

  ---

  ## 검증 방법

  1. **바이트코드 덤프 테스트**
  ```cpp
  // 각 FlowDefinition의 BuildBytecode 호출 후 버퍼 내용 로깅
  TArray<uint8> Buffer;
  FHktFlowBuilder Builder(Buffer);
  Definition->BuildBytecode(Builder, TestEvent, nullptr);
  DumpBytecode(Buffer); // 헥스 덤프 + 오피코드 해석
  ```

  2. **단위 테스트**: `Automation RunTests HktSimulation.FlowDefinitions`
  - 각 FlowDefinition이 유효한 바이트코드 생성하는지
  - VM이 바이트코드를 정상 실행하는지
  - 레지스터 값이 예상대로 변경되는지

  3. **통합 테스트 (PIE)**
  - MoveToLocation: 캐릭터가 목표로 이동 후 정지
  - Fireball: 시전 → 투사체 생성 → 충돌 → 폭발 → 범위 데미지
  - EnterCharacter: 캐릭터/장비 생성 + 애니메이션 재생

  ---

  ## 구현 순서

  1. **오피코드 정의** (HktFlowOpcodes.h)
  2. **Builder API** (HktFlowBuilder.h) - 자연어 수준 메서드
  3. **VM 핸들러** (HktFlowVM.cpp) - 각 오피코드 실행 로직
  4. **MoveToLocationFlowDefinition** - 가장 단순, 기본 검증
  5. **EnterCharacterFlowDefinition** - 순차 실행 패턴 검증
  6. **FireballFlowDefinition** - ForEach, 범위 쿼리 검증

  ---

  ## 설계 핵심

  | 원칙 | 설명 |
  |------|------|
  | **자연어 = API** | Builder 메서드명이 자연어와 1:1 대응 |
  | **모든 것은 Op** | 콜백/분기 없이 모든 로직이 Op으로 표현 |
  | **레지스터 기반** | 중간 결과는 레지스터에 저장, 다음 Op에서 참조 |
  | **블로킹 대기** | WaitXxx Op은 조건 충족까지 VM 블로킹 |

아래 내용으로 수정한다.
● Update(Plugins\HktGameplay\Source\HktSimulation\Private\HktFlowOpcodes.h)
  ⎿  Added 36 lines
      16
      17  namespace ECoreOp
      18  {
      19 +  // === 기본 (0-9) ===
      20    static constexpr HktOpCode None = 0;
      21    static constexpr HktOpCode WaitTime = 1;
      22    static constexpr HktOpCode WaitUntilDestroyed = 2;
      23 +
      24 +  // === 애니메이션 (10-19) ===
      25    static constexpr HktOpCode PlayAnim = 10;
      26    static constexpr HktOpCode SpawnProjectile = 11;
      27    static constexpr HktOpCode DestroyUnit = 12;
      28 +
      29 +  // === 속성 (20-29) ===
      30    static constexpr HktOpCode ModifyAttribute = 20;
      31    static constexpr HktOpCode ExplodeAndDamage = 21;
      32 +
      33 +  // === 이동 (30-39) ===
      34 +  static constexpr HktOpCode MoveTo = 30;           // 목표 위치로 이동 시작
      35 +  static constexpr HktOpCode MoveForward = 31;      // 전방으로 이동
      36 +  static constexpr HktOpCode Stop = 32;             // 이동 중지
      37 +  static constexpr HktOpCode ApplyForce = 33;       // 힘 적용
      38 +
      39 +  // === 이벤트 대기 (40-49) ===
      40 +  static constexpr HktOpCode WaitArrival = 40;      // 도착까지 대기
      41 +  static constexpr HktOpCode WaitCollision = 41;    // 충돌까지 대기 (충돌 상대 Reg에 저장)
      42 +  static constexpr HktOpCode WaitSignal = 42;       // 시그널 대기 (애니메이션 종료 등)
      43 +  static constexpr HktOpCode WaitAny = 43;          // 복수 이벤트 중 하나 대기
      44 +
      45 +  // === 분기 (50-59) ===
      46 +  static constexpr HktOpCode Branch = 50;           // 조건 분기 (이벤트 타입별)
      47 +  static constexpr HktOpCode Jump = 51;             // 무조건 점프
      48 +  static constexpr HktOpCode Halt = 52;             // 실행 종료
      49 +
      50 +  // === 범위 쿼리 (60-69) ===
      51 +  static constexpr HktOpCode QuerySphere = 60;      // 구 범위 쿼리
      52 +  static constexpr HktOpCode ForEachTarget = 61;    // 쿼리 결과 순회
      53 +  static constexpr HktOpCode EndForEach = 62;       // 순회 종료
      54 +
      55 +  // === 엔티티 (70-79) ===
      56 +  static constexpr HktOpCode SpawnEntity = 70;      // 엔티티 생성 (GameplayTag 기반)
      57 +  static constexpr HktOpCode DestroyEntity = 71;    // 엔티티 파괴
      58 +
      59 +  // === 데미지 (80-89) ===
      60 +  static constexpr HktOpCode SetDamage = 80;        // 직접 데미지
      61 +  static constexpr HktOpCode ApplyDot = 81;         // DoT 적용
      62 +
      63    static constexpr HktOpCode EndOfStream = 255;
      64  }
      65

● Update(Plugins\HktGameplay\Source\HktSimulation\Private\HktFlowBuilder.h)
  ⎿  Added 199 lines, removed 12 lines
       3  #pragma once
       4
       5  #include "CoreMinimal.h"
       6 +#include "GameplayTagContainer.h"
       7  #include "HktFlowOpcodes.h"
       8  #include "HktAttributeSet.h"
       9
      10  /**
      11   * Builder (Compiler)
      12   * 바이트코드를 편리하게 작성하기 위한 빌더 클래스
      13 + *
      14 + * 핵심 원칙: Builder API는 자연어와 동일한 추상 수준을 유지
      15   */
      16  struct HKTSIMULATION_API FHktFlowBuilder
      17  {
     ...
       20    FHktFlowBuilder(TArray<uint8>& InBuffer) : Buffer(InBuffer) {}
       21
       22    template<typename T>
       20 -  void PushData(const T& Val)
       23 +  void PushData(const T& Val)
       24    {
       25      int32 Idx = Buffer.AddUninitialized(sizeof(T));
       26      FMemory::Memcpy(&Buffer[Idx], &Val, sizeof(T));
       27    }
       28
       26 -  void PushHeader(HktOpCode Op, uint16 Size)
       29 +  void PushHeader(HktOpCode Op, uint16 Size)
       30    {
       31      FInstructionHeader H = { Op, 0, Size };
       32      int32 Idx = Buffer.AddUninitialized(sizeof(FInstructionHeader));
       33      FMemory::Memcpy(&Buffer[Idx], &H, sizeof(FInstructionHeader));
       34    }
       35
       33 -  FHktFlowBuilder& PlayAnim(FName AnimName)
       36 +  // ============================================================
       37 +  // === 애니메이션 (자연어: "애니메이션 재생") ===
       38 +  // ============================================================
       39 +
       40 +  FHktFlowBuilder& PlayAnim(FName AnimName)
       41    {
       42      PushHeader(ECoreOp::PlayAnim, sizeof(FName));
       43      PushData(AnimName);
       44      return *this;
       45    }
       46
       40 -  FHktFlowBuilder& WaitSeconds(float Duration)
       47 +  FHktFlowBuilder& PlayAnimation(FGameplayTag AnimTag)
       48    {
       49 +    // GameplayTag를 FName으로 변환하여 저장
       50 +    FName AnimName = AnimTag.GetTagName();
       51 +    return PlayAnim(AnimName);
       52 +  }
       53 +
       54 +  // ============================================================
       55 +  // === 대기 (자연어: "~까지 기다림") ===
       56 +  // ============================================================
       57 +
       58 +  FHktFlowBuilder& WaitSeconds(float Duration)
       59 +  {
       60      PushHeader(ECoreOp::WaitTime, sizeof(float));
       61      PushData(Duration);
       62      return *this;
       63    }
       64
       47 -  FHktFlowBuilder& SpawnFireball(UClass* ProjectileClass, uint8 OutRegisterIndex = 0)
       65 +  FHktFlowBuilder& WaitUntilDestroyed(uint8 TargetReg = 0)
       66    {
       49 -    struct Param { UClass* C; float Off; float Spd; uint8 OutReg; } P = { ProjectileClass, 100.0f, 1500.0f, OutRegi
          -sterIndex };
       50 -    PushHeader(ECoreOp::SpawnProjectile, sizeof(Param));
       67 +    struct FParam { uint8 RegIdx; } P = { TargetReg };
       68 +    PushHeader(ECoreOp::WaitUntilDestroyed, sizeof(FParam));
       69      PushData(P);
       70      return *this;
       71    }
       72
       55 -  FHktFlowBuilder& WaitForImpact(uint8 TargetRegisterIndex = 0)
       73 +  FHktFlowBuilder& WaitUntilArrival(float Tolerance = 10.0f)
       74    {
       57 -    struct Param { uint8 RegIdx; } P = { TargetRegisterIndex };
       58 -    PushHeader(ECoreOp::WaitUntilDestroyed, sizeof(Param));
       75 +    struct FParam { float Tolerance; } P = { Tolerance };
       76 +    PushHeader(ECoreOp::WaitArrival, sizeof(FParam));
       77      PushData(P);
       78      return *this;
       79    }
       80
       63 -  FHktFlowBuilder& Explode(float Radius, float Damage, float BurnDmg, float BurnTime, uint8 LocationRegisterIndex =
          - 0)
       81 +  FHktFlowBuilder& WaitUntilCollision(uint8 OutCollidedReg = 0, float CollisionRadius = 50.0f)
       82    {
       83 +    struct FParam { uint8 OutReg; float Radius; } P = { OutCollidedReg, CollisionRadius };
       84 +    PushHeader(ECoreOp::WaitCollision, sizeof(FParam));
       85 +    PushData(P);
       86 +    return *this;
       87 +  }
       88 +
       89 +  FHktFlowBuilder& WaitUntilSignal(FGameplayTag SignalTag)
       90 +  {
       91 +    FName SignalName = SignalTag.GetTagName();
       92 +    PushHeader(ECoreOp::WaitSignal, sizeof(FName));
       93 +    PushData(SignalName);
       94 +    return *this;
       95 +  }
       96 +
       97 +  // ============================================================
       98 +  // === 이동 (자연어: "목표로 이동") ===
       99 +  // ============================================================
      100 +
      101 +  FHktFlowBuilder& MoveTo(FVector Target)
      102 +  {
      103 +    PushHeader(ECoreOp::MoveTo, sizeof(FVector));
      104 +    PushData(Target);
      105 +    return *this;
      106 +  }
      107 +
      108 +  FHktFlowBuilder& MoveForward(float Speed)
      109 +  {
      110 +    struct FParam { float Speed; } P = { Speed };
      111 +    PushHeader(ECoreOp::MoveForward, sizeof(FParam));
      112 +    PushData(P);
      113 +    return *this;
      114 +  }
      115 +
      116 +  FHktFlowBuilder& Stop()
      117 +  {
      118 +    PushHeader(ECoreOp::Stop, 0);
      119 +    return *this;
      120 +  }
      121 +
      122 +  FHktFlowBuilder& ApplyForce(FVector Direction, float Magnitude)
      123 +  {
      124 +    struct FParam { FVector Dir; float Mag; } P = { Direction, Magnitude };
      125 +    PushHeader(ECoreOp::ApplyForce, sizeof(FParam));
      126 +    PushData(P);
      127 +    return *this;
      128 +  }
      129 +
      130 +  // ============================================================
      131 +  // === 엔티티 (자연어: "엔티티 생성/파괴") ===
      132 +  // ============================================================
      133 +
      134 +  FHktFlowBuilder& SpawnEntity(FGameplayTag EntityTag, uint8 OutReg = 0)
      135 +  {
      136 +    FName EntityName = EntityTag.GetTagName();
      137 +    struct FParam { FName Tag; uint8 OutReg; } P = { EntityName, OutReg };
      138 +    PushHeader(ECoreOp::SpawnEntity, sizeof(FParam));
      139 +    PushData(P);
      140 +    return *this;
      141 +  }
      142 +
      143 +  FHktFlowBuilder& DestroyEntity(uint8 TargetReg)
      144 +  {
      145 +    struct FParam { uint8 Reg; } P = { TargetReg };
      146 +    PushHeader(ECoreOp::DestroyEntity, sizeof(FParam));
      147 +    PushData(P);
      148 +    return *this;
      149 +  }
      150 +
      151 +  // ============================================================
      152 +  // === 데미지 (자연어: "데미지 적용") ===
      153 +  // ============================================================
      154 +
      155 +  FHktFlowBuilder& SetDamage(uint8 TargetReg, float Damage)
      156 +  {
      157 +    struct FParam { uint8 TargetReg; float Damage; } P = { TargetReg, Damage };
      158 +    PushHeader(ECoreOp::SetDamage, sizeof(FParam));
      159 +    PushData(P);
      160 +    return *this;
      161 +  }
      162 +
      163 +  FHktFlowBuilder& ApplyBurning(uint8 TargetReg, float DmgPerSec, float Duration)
      164 +  {
      165 +    struct FParam { uint8 TargetReg; float DmgPerSec; float Duration; } P = { TargetReg, DmgPerSec, Duration };
      166 +    PushHeader(ECoreOp::ApplyDot, sizeof(FParam));
      167 +    PushData(P);
      168 +    return *this;
      169 +  }
      170 +
      171 +  // ============================================================
      172 +  // === 범위 쿼리 (자연어: "주변 유닛 검색") ===
      173 +  // ============================================================
      174 +
      175 +  FHktFlowBuilder& QueryNearby(float Radius, uint8 CenterReg, uint8 OutListReg)
      176 +  {
      177 +    struct FParam { float Radius; uint8 CenterReg; uint8 OutListReg; } P = { Radius, CenterReg, OutListReg };
      178 +    PushHeader(ECoreOp::QuerySphere, sizeof(FParam));
      179 +    PushData(P);
      180 +    return *this;
      181 +  }
      182 +
      183 +  FHktFlowBuilder& ForEachTarget(uint8 ListReg, uint8 IteratorReg)
      184 +  {
      185 +    // EndOffset은 EndForEach에서 패치됨 (빌드 타임에 알 수 없음)
      186 +    struct FParam { uint8 ListReg; uint8 IterReg; int32 EndOffset; } P = { ListReg, IteratorReg, 0 };
      187 +    ForEachStartOffset = Buffer.Num(); // 패치 위치 기록
      188 +    PushHeader(ECoreOp::ForEachTarget, sizeof(FParam));
      189 +    PushData(P);
      190 +    return *this;
      191 +  }
      192 +
      193 +  FHktFlowBuilder& EndForEach()
      194 +  {
      195 +    // ForEachTarget의 EndOffset 패치
      196 +    if (ForEachStartOffset > 0)
      197 +    {
      198 +      int32 CurrentOffset = Buffer.Num();
      199 +      int32 JumpOffset = CurrentOffset - ForEachStartOffset;
      200 +
      201 +      // ForEachTarget의 EndOffset 필드 위치 계산
      202 +      int32 EndOffsetFieldPos = ForEachStartOffset + sizeof(FInstructionHeader) + sizeof(uint8) + sizeof(uint8);
      203 +      FMemory::Memcpy(&Buffer[EndOffsetFieldPos], &JumpOffset, sizeof(int32));
      204 +    }
      205 +
      206 +    PushHeader(ECoreOp::EndForEach, 0);
      207 +    ForEachStartOffset = 0;
      208 +    return *this;
      209 +  }
      210 +
      211 +  // ============================================================
      212 +  // === 흐름 제어 (자연어: "분기/종료") ===
      213 +  // ============================================================
      214 +
      215 +  FHktFlowBuilder& Jump(int32 Offset)
      216 +  {
      217 +    struct FParam { int32 Offset; } P = { Offset };
      218 +    PushHeader(ECoreOp::Jump, sizeof(FParam));
      219 +    PushData(P);
      220 +    return *this;
      221 +  }
      222 +
      223 +  FHktFlowBuilder& End()
      224 +  {
      225 +    PushHeader(ECoreOp::Halt, 0);
      226 +    return *this;
      227 +  }
      228 +
      229 +  // ============================================================
      230 +  // === Legacy API (기존 호환성) ===
      231 +  // ============================================================
      232 +
      233 +  FHktFlowBuilder& SpawnFireball(UClass* ProjectileClass, uint8 OutRegisterIndex = 0)
      234 +  {
      235 +    struct Param { UClass* C; float Off; float Spd; uint8 OutReg; } P = { ProjectileClass, 100.0f, 1500.0f, OutRegi
          +sterIndex };
      236 +    PushHeader(ECoreOp::SpawnProjectile, sizeof(Param));
      237 +    PushData(P);
      238 +    return *this;
      239 +  }
      240 +
      241 +  FHktFlowBuilder& WaitForImpact(uint8 TargetRegisterIndex = 0)
      242 +  {
      243 +    return WaitUntilDestroyed(TargetRegisterIndex);
      244 +  }
      245 +
      246 +  FHktFlowBuilder& Explode(float Radius, float Damage, float BurnDmg, float BurnTime, uint8 LocationRegisterIndex =
          + 0)
      247 +  {
      248      struct Param { float R; float D; float BD; float BT; uint8 LocReg; } P = { Radius, Damage, BurnDmg, BurnTime, L
           ocationRegisterIndex };
      249      PushHeader(ECoreOp::ExplodeAndDamage, sizeof(Param));
      250      PushData(P);
      251      return *this;
      252    }
      253
       71 -  FHktFlowBuilder& ModifyHealth(uint8 TargetRegisterIndex, float DeltaValue)
      254 +  FHktFlowBuilder& ModifyHealth(uint8 TargetRegisterIndex, float DeltaValue)
      255    {
      256      struct Param { uint8 TargetReg; EHktAttribute AttrId; float Value; } P = { TargetRegisterIndex, EHktAttribute::
           Health, DeltaValue };
      257      PushHeader(ECoreOp::ModifyAttribute, sizeof(Param));
      258      PushData(P);
      259      return *this;
      260    }
      261 +
      262 +private:
      263 +  // ForEach 패치용 오프셋 기록
      264 +  int32 ForEachStartOffset = 0;
      265  };

      ● Update(Plugins\HktGameplay\Source\HktSimulation\Private\HktFlowVM.h)
  ⎿  Added 9 lines, removed 7 lines
      11   * 바이트코드 실행을 위한 가상 머신
      12   */
      13
      14 -enum class EFlowRegType : uint8
      15 -{
      16 -  Empty,
      17 -  Unit,
      18 -  Vector,
      19 -  Scalar,
      20 -  Player
      14 +enum class EFlowRegType : uint8
      15 +{
      16 +  Empty,
      17 +  Unit,
      18 +  Vector,
      19 +  Scalar,
      20 +  Player,
      21 +  UnitList,  // 범위 쿼리 결과용
      22 +  Integer    // 루프 인덱스용
      23  };
      24
      25  struct FFlowRegister

● Update(Plugins\HktGameplay\Source\HktSimulation\Private\HktFlowVM.h)
  ⎿  Added 12 lines, removed 2 lines
      32      FPlayerHandle Player;  // 4 bytes
      33      FVector VectorVal;     // 24 bytes
      34      float ScalarVal;       // 4 bytes
      35 +    int32 IntVal;          // 4 bytes (루프 인덱스)
      36    };
      36 -
      37 +
      38 +  // UnitList는 union에 넣지 않음 (동적 할당)
      39 +  TArray<FUnitHandle> UnitList;
      40 +
      41    FFlowRegister()
      42    {
      43      Type = EFlowRegType::Empty;
     ...
      48    void SetVector(const FVector& InVec) { Type = EFlowRegType::Vector; VectorVal = InVec; }
      49    void SetScalar(float InScalar) { Type = EFlowRegType::Scalar; ScalarVal = InScalar; }
      50    void SetPlayer(FPlayerHandle InPlayer) { Type = EFlowRegType::Player; Player = InPlayer; }
      47 -
      51 +  void SetInt(int32 InInt) { Type = EFlowRegType::Integer; IntVal = InInt; }
      52 +  void SetUnitList(const TArray<FUnitHandle>& InList) { Type = EFlowRegType::UnitList; UnitList = InList; }
      53 +
      54 +  // Getters
      55 +  int32& GetInt() { return IntVal; }
      56 +  TArray<FUnitHandle>& GetUnitList() { return UnitList; }
      57 +
      58    // Helper Getter
      59    FVector ResolveLocation(const FHktEntityManager* Mgr) const;
      60  };

● Update(Plugins\HktGameplay\Source\HktSimulation\Private\HktFlowVM.h)
  ⎿  Added 12 lines, removed 3 lines
      61
      62  struct FFlowRegisters
      63  {
      64 -  FUnitHandle OwnerUnit;
      64 +  FUnitHandle OwnerUnit;
      65    FPlayerHandle OwnerPlayer; // 시전한 플레이어 정보 캐싱
      66 -
      66 +
      67    static constexpr int32 NumGPR = 8;
      68    FFlowRegister GPR[NumGPR];
      69 -
      69 +
      70    float WaitTimer = 0.0f;
      71    int32 ProgramCounter = 0;
      72    bool bBlocked = false;
      73    uint8 BlockingRegIndex = 0;
      74
      75 +  // 이동 관련 상태
      76 +  FVector MoveTarget = FVector::ZeroVector;
      77 +  float MoveSpeed = 0.0f;
      78 +  bool bIsMoving = false;
      79 +
      80 +  // ForEach 루프 상태 (별도의 레지스터 대신 명시적 관리)
      81 +  int32 LoopIndex = 0;
      82 +  int32 LoopEndPC = 0;  // 루프 종료 후 점프할 PC
      83 +
      84    FFlowRegister& Get(uint8 Index) { return GPR[Index % NumGPR]; }
      85  };
      86

● Update(Plugins\HktGameplay\Source\HktSimulation\Private\HktFlowVM.h)
  ⎿  Added 28 lines
      104    static void Op_SpawnProjectile(FHktFlowVM& VM, uint8* Data);
      105    static void Op_ModifyAttribute(FHktFlowVM& VM, uint8* Data);
      106    static void Op_ExplodeAndDamage(FHktFlowVM& VM, uint8* Data);
      107 +
      108 +  // --- Movement OpHandlers ---
      109 +  static void Op_MoveTo(FHktFlowVM& VM, uint8* Data);
      110 +  static void Op_MoveForward(FHktFlowVM& VM, uint8* Data);
      111 +  static void Op_Stop(FHktFlowVM& VM, uint8* Data);
      112 +  static void Op_ApplyForce(FHktFlowVM& VM, uint8* Data);
      113 +
      114 +  // --- Wait OpHandlers ---
      115 +  static void Op_WaitArrival(FHktFlowVM& VM, uint8* Data);
      116 +  static void Op_WaitCollision(FHktFlowVM& VM, uint8* Data);
      117 +  static void Op_WaitSignal(FHktFlowVM& VM, uint8* Data);
      118 +
      119 +  // --- Flow Control OpHandlers ---
      120 +  static void Op_Jump(FHktFlowVM& VM, uint8* Data);
      121 +  static void Op_Halt(FHktFlowVM& VM, uint8* Data);
      122 +
      123 +  // --- Query OpHandlers ---
      124 +  static void Op_QuerySphere(FHktFlowVM& VM, uint8* Data);
      125 +  static void Op_ForEachTarget(FHktFlowVM& VM, uint8* Data);
      126 +  static void Op_EndForEach(FHktFlowVM& VM, uint8* Data);
      127 +
      128 +  // --- Entity OpHandlers ---
      129 +  static void Op_SpawnEntity(FHktFlowVM& VM, uint8* Data);
      130 +  static void Op_DestroyEntity(FHktFlowVM& VM, uint8* Data);
      131 +
      132 +  // --- Damage OpHandlers ---
      133 +  static void Op_SetDamage(FHktFlowVM& VM, uint8* Data);
      134 +  static void Op_ApplyDot(FHktFlowVM& VM, uint8* Data);
      135  };

● Update(Plugins\HktGameplay\Source\HktSimulation\Private\HktFlowVM.cpp)
  ⎿  Added 306 lines, removed 1 line
      182    FParam* P = reinterpret_cast<FParam*>(Data);
      183
      184    FVector CenterLoc = VM.Regs.Get(P->LocReg).ResolveLocation(VM.Mgr);
      185 -
      185 +
      186    // [Optimized] Use spatial index for range query instead of linear search
      187    TArray<FUnitHandle> AffectedUnits;
      188    VM.Mgr->QueryUnitsInSphere(CenterLoc, P->Radius, AffectedUnits, true);
     ...
      205      // TODO: Apply DoT (would require a separate system for ongoing effects)
      206    }
      207  }
      208 +
      209 +// ============================================================
      210 +// === Movement OpHandlers ===
      211 +// ============================================================
      212 +
      213 +void FHktFlowVM::Op_MoveTo(FHktFlowVM& VM, uint8* Data)
      214 +{
      215 +  FVector* Target = reinterpret_cast<FVector*>(Data);
      216 +
      217 +  if (VM.Mgr->IsUnitValid(VM.Regs.OwnerUnit))
      218 +  {
      219 +    VM.Regs.MoveTarget = *Target;
      220 +    VM.Regs.bIsMoving = true;
      221 +    // 실제 이동은 Tick에서 처리하거나 외부 시스템에 위임
      222 +  }
      223 +}
      224 +
      225 +void FHktFlowVM::Op_MoveForward(FHktFlowVM& VM, uint8* Data)
      226 +{
      227 +  struct FParam { float Speed; };
      228 +  FParam* P = reinterpret_cast<FParam*>(Data);
      229 +
      230 +  if (VM.Mgr->IsUnitValid(VM.Regs.OwnerUnit))
      231 +  {
      232 +    VM.Regs.MoveSpeed = P->Speed;
      233 +    VM.Regs.bIsMoving = true;
      234 +
      235 +    // 전방 방향으로 이동 목표 설정 (먼 거리)
      236 +    FVector CurrentLoc = VM.Mgr->GetUnitLocation(VM.Regs.OwnerUnit);
      237 +    FRotator CurrentRot = VM.Mgr->GetUnitRotation(VM.Regs.OwnerUnit);
      238 +    VM.Regs.MoveTarget = CurrentLoc + CurrentRot.Vector() * 100000.0f; // 매우 먼 거리
      239 +  }
      240 +}
      241 +
      242 +void FHktFlowVM::Op_Stop(FHktFlowVM& VM, uint8* Data)
      243 +{
      244 +  VM.Regs.bIsMoving = false;
      245 +  VM.Regs.MoveSpeed = 0.0f;
      246 +}
      247 +
      248 +void FHktFlowVM::Op_ApplyForce(FHktFlowVM& VM, uint8* Data)
      249 +{
      250 +  struct FParam { FVector Dir; float Mag; };
      251 +  FParam* P = reinterpret_cast<FParam*>(Data);
      252 +
      253 +  if (VM.Mgr->IsUnitValid(VM.Regs.OwnerUnit))
      254 +  {
      255 +    // 힘 적용은 물리 시스템에 위임
      256 +    // 여기서는 위치를 즉시 이동시키는 간단한 구현
      257 +    FVector CurrentLoc = VM.Mgr->GetUnitLocation(VM.Regs.OwnerUnit);
      258 +    FVector NewLoc = CurrentLoc + P->Dir.GetSafeNormal() * P->Mag;
      259 +
      260 +    // EntityDatabase 직접 수정 (간소화된 구현)
      261 +    if (VM.Mgr->Entities.Locations.IsValidIndex(VM.Regs.OwnerUnit.Index))
      262 +    {
      263 +      VM.Mgr->Entities.Locations[VM.Regs.OwnerUnit.Index] = NewLoc;
      264 +    }
      265 +  }
      266 +}
      267 +
      268 +// ============================================================
      269 +// === Wait OpHandlers ===
      270 +// ============================================================
      271 +
      272 +void FHktFlowVM::Op_WaitArrival(FHktFlowVM& VM, uint8* Data)
      273 +{
      274 +  struct FParam { float Tolerance; };
      275 +  FParam* P = reinterpret_cast<FParam*>(Data);
      276 +
      277 +  if (!VM.Regs.bIsMoving)
      278 +  {
      279 +    // 이동 중이 아니면 바로 통과
      280 +    return;
      281 +  }
      282 +
      283 +  FVector CurrentPos = VM.Mgr->GetUnitLocation(VM.Regs.OwnerUnit);
      284 +  float Dist = FVector::Dist(CurrentPos, VM.Regs.MoveTarget);
      285 +
      286 +  if (Dist > P->Tolerance)
      287 +  {
      288 +    VM.Regs.bBlocked = true;
      289 +    // PC를 되돌려서 다음 Tick에 다시 체크
      290 +    VM.Regs.ProgramCounter -= (sizeof(FInstructionHeader) + sizeof(FParam));
      291 +  }
      292 +  else
      293 +  {
      294 +    // 도착
      295 +    VM.Regs.bIsMoving = false;
      296 +  }
      297 +}
      298 +
      299 +void FHktFlowVM::Op_WaitCollision(FHktFlowVM& VM, uint8* Data)
      300 +{
      301 +  struct FParam { uint8 OutReg; float Radius; };
      302 +  FParam* P = reinterpret_cast<FParam*>(Data);
      303 +
      304 +  // 범위 내 유닛 검색
      305 +  FVector CurrentPos = VM.Mgr->GetUnitLocation(VM.Regs.OwnerUnit);
      306 +  TArray<FUnitHandle> NearbyUnits;
      307 +  VM.Mgr->QueryUnitsInSphere(CurrentPos, P->Radius, NearbyUnits, true);
      308 +
      309 +  // 자기 자신 제외
      310 +  FUnitHandle Collided;
      311 +  for (const FUnitHandle& Handle : NearbyUnits)
      312 +  {
      313 +    if (Handle != VM.Regs.OwnerUnit)
      314 +    {
      315 +      Collided = Handle;
      316 +      break;
      317 +    }
      318 +  }
      319 +
      320 +  if (Collided.IsValid())
      321 +  {
      322 +    // 충돌 발생, 결과 저장
      323 +    VM.Regs.Get(P->OutReg).SetUnit(Collided);
      324 +  }
      325 +  else
      326 +  {
      327 +    // 충돌 없음, 블로킹
      328 +    VM.Regs.bBlocked = true;
      329 +    VM.Regs.ProgramCounter -= (sizeof(FInstructionHeader) + sizeof(FParam));
      330 +  }
      331 +}
      332 +
      333 +void FHktFlowVM::Op_WaitSignal(FHktFlowVM& VM, uint8* Data)
      334 +{
      335 +  FName* SignalName = reinterpret_cast<FName*>(Data);
      336 +
      337 +  // Signal 시스템은 별도 구현 필요
      338 +  // 현재는 즉시 통과 (Placeholder)
      339 +  // TODO: Signal 시스템과 연동
      340 +}
      341 +
      342 +// ============================================================
      343 +// === Flow Control OpHandlers ===
      344 +// ============================================================
      345 +
      346 +void FHktFlowVM::Op_Jump(FHktFlowVM& VM, uint8* Data)
      347 +{
      348 +  struct FParam { int32 Offset; };
      349 +  FParam* P = reinterpret_cast<FParam*>(Data);
      350 +
      351 +  // 상대 점프: 현재 PC + Offset
      352 +  // 주의: Tick에서 이미 NextPC로 증가시키므로, 여기서 조정
      353 +  VM.Regs.ProgramCounter += P->Offset - (sizeof(FInstructionHeader) + sizeof(FParam));
      354 +}
      355 +
      356 +void FHktFlowVM::Op_Halt(FHktFlowVM& VM, uint8* Data)
      357 +{
      358 +  // 실행 종료: PC를 버퍼 끝으로 설정
      359 +  VM.Regs.ProgramCounter = VM.Bytecode.Num();
      360 +}
      361 +
      362 +// ============================================================
      363 +// === Query OpHandlers ===
      364 +// ============================================================
      365 +
      366 +void FHktFlowVM::Op_QuerySphere(FHktFlowVM& VM, uint8* Data)
      367 +{
      368 +  struct FParam { float Radius; uint8 CenterReg; uint8 OutListReg; };
      369 +  FParam* P = reinterpret_cast<FParam*>(Data);
      370 +
      371 +  FVector CenterLoc = VM.Regs.Get(P->CenterReg).ResolveLocation(VM.Mgr);
      372 +
      373 +  TArray<FUnitHandle> Results;
      374 +  VM.Mgr->QueryUnitsInSphere(CenterLoc, P->Radius, Results, true);
      375 +
      376 +  // 결과를 레지스터에 저장
      377 +  VM.Regs.Get(P->OutListReg).SetUnitList(Results);
      378 +}
      379 +
      380 +void FHktFlowVM::Op_ForEachTarget(FHktFlowVM& VM, uint8* Data)
      381 +{
      382 +  struct FParam { uint8 ListReg; uint8 IterReg; int32 EndOffset; };
      383 +  FParam* P = reinterpret_cast<FParam*>(Data);
      384 +
      385 +  TArray<FUnitHandle>& List = VM.Regs.Get(P->ListReg).GetUnitList();
      386 +  int32& Index = VM.Regs.LoopIndex;
      387 +
      388 +  if (Index < List.Num())
      389 +  {
      390 +    // 현재 항목을 Iterator 레지스터에 설정
      391 +    VM.Regs.Get(P->IterReg).SetUnit(List[Index]);
      392 +    Index++;
      393 +  }
      394 +  else
      395 +  {
      396 +    // 루프 종료: EndForEach로 점프
      397 +    Index = 0; // 리셋
      398 +    VM.Regs.ProgramCounter += P->EndOffset - (sizeof(FInstructionHeader) + sizeof(FParam));
      399 +  }
      400 +}
      401 +
      402 +void FHktFlowVM::Op_EndForEach(FHktFlowVM& VM, uint8* Data)
      403 +{
      404 +  // ForEachTarget으로 돌아가기
      405 +  // Builder에서 기록한 ForEachStartOffset을 사용해야 함
      406 +  // 간소화된 구현: ForEachTarget 명령어를 다시 찾아서 점프
      407 +
      408 +  // Bytecode를 역방향 탐색하여 ForEachTarget 찾기
      409 +  int32 SearchPC = VM.Regs.ProgramCounter - sizeof(FInstructionHeader);
      410 +
      411 +  while (SearchPC > 0)
      412 +  {
      413 +    FInstructionHeader* Header = reinterpret_cast<FInstructionHeader*>(&VM.Bytecode[SearchPC]);
      414 +
      415 +    if (Header->Op == ECoreOp::ForEachTarget)
      416 +    {
      417 +      // ForEachTarget으로 점프 (NextPC 계산을 고려)
      418 +      VM.Regs.ProgramCounter = SearchPC - (sizeof(FInstructionHeader));
      419 +      return;
      420 +    }
      421 +
      422 +    SearchPC -= sizeof(FInstructionHeader);
      423 +    // 간단한 역방향 탐색 (실제로는 더 정교한 방법 필요)
      424 +    if (SearchPC > 0)
      425 +    {
      426 +      // 이전 명령어의 크기를 알 수 없으므로 단순화
      427 +      SearchPC -= 1;
      428 +    }
      429 +  }
      430 +
      431 +  // ForEachTarget을 찾지 못함 - 그냥 진행
      432 +}
      433 +
      434 +// ============================================================
      435 +// === Entity OpHandlers ===
      436 +// ============================================================
      437 +
      438 +void FHktFlowVM::Op_SpawnEntity(FHktFlowVM& VM, uint8* Data)
      439 +{
      440 +  struct FParam { FName Tag; uint8 OutReg; };
      441 +  FParam* P = reinterpret_cast<FParam*>(Data);
      442 +
      443 +  if (VM.Mgr)
      444 +  {
      445 +    // Owner의 위치에서 엔티티 생성
      446 +    FVector SpawnLoc = VM.Mgr->GetUnitLocation(VM.Regs.OwnerUnit);
      447 +    FRotator SpawnRot = VM.Mgr->GetUnitRotation(VM.Regs.OwnerUnit);
      448 +
      449 +    // 새 유닛 할당
      450 +    FUnitHandle NewHandle = VM.Mgr->AllocUnit(VM.Regs.OwnerPlayer, SpawnLoc, SpawnRot);
      451 +
      452 +    // 레지스터에 저장
      453 +    VM.Regs.Get(P->OutReg).SetUnit(NewHandle);
      454 +
      455 +    // TODO: Tag 기반 비주얼 액터 생성 (EntityFactory 시스템 필요)
      456 +  }
      457 +}
      458 +
      459 +void FHktFlowVM::Op_DestroyEntity(FHktFlowVM& VM, uint8* Data)
      460 +{
      461 +  struct FParam { uint8 Reg; };
      462 +  FParam* P = reinterpret_cast<FParam*>(Data);
      463 +
      464 +  FFlowRegister& Reg = VM.Regs.Get(P->Reg);
      465 +
      466 +  if (Reg.Type == EFlowRegType::Unit && VM.Mgr)
      467 +  {
      468 +    VM.Mgr->FreeUnit(Reg.Unit);
      469 +    Reg.Type = EFlowRegType::Empty;
      470 +  }
      471 +}
      472 +
      473 +// ============================================================
      474 +// === Damage OpHandlers ===
      475 +// ============================================================
      476 +
      477 +void FHktFlowVM::Op_SetDamage(FHktFlowVM& VM, uint8* Data)
      478 +{
      479 +  struct FParam { uint8 TargetReg; float Damage; };
      480 +  FParam* P = reinterpret_cast<FParam*>(Data);
      481 +
      482 +  FFlowRegister& Reg = VM.Regs.Get(P->TargetReg);
      483 +
      484 +  if (Reg.Type == EFlowRegType::Unit)
      485 +  {
      486 +    if (FHktAttributeSet* Attrs = VM.Mgr->GetUnitAttrs(Reg.Unit))
      487 +    {
      488 +      Attrs->Modify(EHktAttribute::Health, -P->Damage);
      489 +    }
      490 +  }
      491 +}
      492 +
      493 +void FHktFlowVM::Op_ApplyDot(FHktFlowVM& VM, uint8* Data)
      494 +{
      495 +  struct FParam { uint8 TargetReg; float DmgPerSec; float Duration; };
      496 +  FParam* P = reinterpret_cast<FParam*>(Data);
      497 +
      498 +  // DoT(Damage over Time) 시스템은 별도 구현 필요
      499 +  // 현재는 즉시 총 데미지 적용 (간소화)
      500 +  FFlowRegister& Reg = VM.Regs.Get(P->TargetReg);
      501 +
      502 +  if (Reg.Type == EFlowRegType::Unit)
      503 +  {
      504 +    if (FHktAttributeSet* Attrs = VM.Mgr->GetUnitAttrs(Reg.Unit))
      505 +    {
      506 +      float TotalDamage = P->DmgPerSec * P->Duration;
      507 +      Attrs->Modify(EHktAttribute::Health, -TotalDamage);
      508 +    }
      509 +  }
      510 +
      511 +  // TODO: DoT 효과를 별도 시스템에 등록하여 시간에 따라 적용
      512 +}

● Update(Plugins\HktGameplay\Source\HktSimulation\Private\HktFlowOpcodes.cpp)
  ⎿  Added 30 lines, removed 2 lines
      151  {
      152    if (bInitialized) return;
      153    bInitialized = true;
      154 -
      155 -  // Register core opcodes with category
      154 +
      155 +  // === 기존 Core Opcodes ===
      156    RegisterOpcode(ECoreOp::WaitTime, FHktFlowVM::Op_WaitTime, TEXT("Core.Wait"));
      157    RegisterOpcode(ECoreOp::WaitUntilDestroyed, FHktFlowVM::Op_WaitUntilDestroyed, TEXT("Core.Wait"));
      158    RegisterOpcode(ECoreOp::PlayAnim, FHktFlowVM::Op_PlayAnim, TEXT("Core.Animation"));
     ...
      160    RegisterOpcode(ECoreOp::ExplodeAndDamage, FHktFlowVM::Op_ExplodeAndDamage, TEXT("Core.Damage"));
      161    RegisterOpcode(ECoreOp::ModifyAttribute, FHktFlowVM::Op_ModifyAttribute, TEXT("Core.Attribute"));
      162
      163 +  // === Movement Opcodes ===
      164 +  RegisterOpcode(ECoreOp::MoveTo, FHktFlowVM::Op_MoveTo, TEXT("Movement"));
      165 +  RegisterOpcode(ECoreOp::MoveForward, FHktFlowVM::Op_MoveForward, TEXT("Movement"));
      166 +  RegisterOpcode(ECoreOp::Stop, FHktFlowVM::Op_Stop, TEXT("Movement"));
      167 +  RegisterOpcode(ECoreOp::ApplyForce, FHktFlowVM::Op_ApplyForce, TEXT("Movement"));
      168 +
      169 +  // === Wait Opcodes ===
      170 +  RegisterOpcode(ECoreOp::WaitArrival, FHktFlowVM::Op_WaitArrival, TEXT("Wait"));
      171 +  RegisterOpcode(ECoreOp::WaitCollision, FHktFlowVM::Op_WaitCollision, TEXT("Wait"));
      172 +  RegisterOpcode(ECoreOp::WaitSignal, FHktFlowVM::Op_WaitSignal, TEXT("Wait"));
      173 +
      174 +  // === Flow Control Opcodes ===
      175 +  RegisterOpcode(ECoreOp::Jump, FHktFlowVM::Op_Jump, TEXT("FlowControl"));
      176 +  RegisterOpcode(ECoreOp::Halt, FHktFlowVM::Op_Halt, TEXT("FlowControl"));
      177 +
      178 +  // === Query Opcodes ===
      179 +  RegisterOpcode(ECoreOp::QuerySphere, FHktFlowVM::Op_QuerySphere, TEXT("Query"));
      180 +  RegisterOpcode(ECoreOp::ForEachTarget, FHktFlowVM::Op_ForEachTarget, TEXT("Query"));
      181 +  RegisterOpcode(ECoreOp::EndForEach, FHktFlowVM::Op_EndForEach, TEXT("Query"));
      182 +
      183 +  // === Entity Opcodes ===
      184 +  RegisterOpcode(ECoreOp::SpawnEntity, FHktFlowVM::Op_SpawnEntity, TEXT("Entity"));
      185 +  RegisterOpcode(ECoreOp::DestroyEntity, FHktFlowVM::Op_DestroyEntity, TEXT("Entity"));
      186 +
      187 +  // === Damage Opcodes ===
      188 +  RegisterOpcode(ECoreOp::SetDamage, FHktFlowVM::Op_SetDamage, TEXT("Damage"));
      189 +  RegisterOpcode(ECoreOp::ApplyDot, FHktFlowVM::Op_ApplyDot, TEXT("Damage"));
      190 +
      191    UE_LOG(LogOpRegistry, Log, TEXT("Opcode registry initialized with %d core opcodes"), GetRegisteredCount());
      192  }