#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

// ============================================================================
// HktVM Types - 기본 타입 정의
// 
// 설계 철학:
// - 자연어의 시간-공간 연속성을 VM이 추상화
// - Flow 작성자는 선형 흐름만 기술, 콜백/이벤트 없음
// - DOD: 캐시 적중률 최대화, 고정 크기 구조체
// ============================================================================

// 캐시 라인 크기 (대부분 CPU에서 64바이트)
constexpr int32 HKT_CACHE_LINE = 64;

// VM 배치 크기 (캐시 라인의 배수)
constexpr int32 HKT_VM_BATCH_SIZE = 64;

// 레지스터 개수 (고정)
constexpr int32 HKT_NUM_REGISTERS = 16;

// 태그 테이블 최대 크기
constexpr int32 HKT_MAX_TAGS = 64;

// 상수 풀 최대 크기
constexpr int32 HKT_MAX_CONSTANTS = 128;

// ============================================================================
// OpCode - 명령어 (무엇을)
// 
// 각 명령어는 자연어의 동사에 대응:
// - SpawnEntity: "~을 생성한다"
// - MoveForward: "~방향으로 이동한다"
// - WaitUntilCollision: "~에 닿을 때까지 기다린다"
// ============================================================================

enum class EHktOp : uint8
{
    Nop = 0,                // 아무것도 하지 않음
    
    // === 시간 제어 (Yield Points) ===
    // 자연어의 시간 흐름을 VM이 추상화
    Yield,                  // 다음 틱까지 양보
    WaitTime,               // N초 동안 대기
    WaitCollision,          // 충돌할 때까지 대기
    WaitArrival,            // 도착할 때까지 대기
    WaitCondition,          // 조건 충족까지 대기
    
    // === 엔티티 동작 ===
    Spawn,                  // 엔티티 생성 -> 레지스터에 저장
    Destroy,                // 엔티티 제거
    
    // === 이동 ===
    MoveForward,            // 전방 이동 (속도)
    MoveTo,                 // 위치로 이동
    Stop,                   // 정지
    
    // === 애니메이션/효과 ===
    PlayAnim,               // 애니메이션 재생
    PlayEffect,             // 이펙트 재생
    
    // === 전투 ===
    Damage,                 // 데미지 적용
    ApplyStatus,            // 상태이상 적용
    
    // === 공간 쿼리 ===
    QueryRadius,            // 반경 내 엔티티 쿼리
    QueryRay,               // 레이캐스트
    
    // === 분기 (언제) ===
    Jump,                   // 무조건 점프
    JumpIfZero,             // 레지스터가 0이면 점프
    JumpIfNotZero,          // 레지스터가 0이 아니면 점프
    JumpIfLess,             // R[A] < R[B] 이면 점프
    JumpIfGreater,          // R[A] > R[B] 이면 점프
    
    // === 태그 기반 분기 (NEW) ===
    JumpIfHasTag,           // 엔티티가 태그를 가지면 점프
    JumpIfNotHasTag,        // 엔티티가 태그를 안가지면 점프
    JumpIfProcessDoing,     // 프로세스가 태그 작업 중이면 점프
    JumpIfProcessDone,      // 프로세스가 태그 작업 완료면 점프
    
    // === 루프 ===
    ForEach,                // 리스트 순회 시작
    EndForEach,             // 리스트 순회 끝
    
    // === 레지스터 조작 ===
    LoadConst,              // 상수 풀에서 레지스터로 로드
    LoadAttr,               // 속성에서 레지스터로 로드 (enum 기반 - deprecated)
    StoreAttr,              // 레지스터에서 속성으로 저장 (enum 기반 - deprecated)
    Copy,                   // 레지스터 복사
    
    // === 태그 기반 속성 (NEW) ===
    LoadAttrByTag,          // 태그로 속성 로드 (유연한 접근)
    StoreAttrByTag,         // 태그로 속성 저장 (유연한 접근)
    AddAttrByTag,           // 태그 속성에 값 더하기
    
    // === 태그 조작 (NEW) ===
    AddStatusTag,           // 엔티티에 상태 태그 추가
    RemoveStatusTag,        // 엔티티에서 상태 태그 제거
    AddProcessTag,          // 프로세스에 진행/완료 태그 추가
    RemoveProcessTag,       // 프로세스에서 태그 제거
    MarkProcessDoing,       // 프로세스 Doing 태그 설정
    MarkProcessDone,        // 프로세스 Done 태그로 이동
    
    // === 산술 연산 ===
    Add,                    // R[C] = R[A] + R[B]
    Sub,                    // R[C] = R[A] - R[B]
    Mul,                    // R[C] = R[A] * R[B]
    Div,                    // R[C] = R[A] / R[B]
    
    // === 종료 ===
    End,                    // Flow 종료
    
    MAX
};

// ============================================================================
// VM 상태
// ============================================================================

enum class EHktVMState : uint8
{
    Ready = 0,              // 실행 준비됨
    Running,                // 실행 중
    Yielded,                // 양보됨 (다음 틱에 재개)
    WaitingTime,            // 시간 대기 중
    WaitingCondition,       // 조건 대기 중
    Finished,               // 완료됨
    Error,                  // 에러 발생
};

// ============================================================================
// Yield 조건 타입
// ============================================================================

enum class EHktYieldCondition : uint8
{
    None = 0,
    Time,                   // 시간 경과
    Collision,              // 충돌 발생
    Arrival,                // 목적지 도착
    Custom,                 // 커스텀 조건
};

// ============================================================================
// 레지스터 (휘발성) - 64비트 Union
// 
// DOD: 고정 크기로 캐시 예측 가능
// 타입 안전성은 포기, 성능 우선
// ============================================================================

union FHktRegister
{
    int64   I64;            // 정수 (64비트)
    double  F64;            // 실수 (64비트)
    float   F32[2];         // 2D 벡터
    int32   I32[2];         // 정수 페어 (EntityID, Generation)
    uint64  Raw;            // Raw 비트
    
    // 편의 접근자
    FORCEINLINE int32 AsInt() const { return static_cast<int32>(I64); }
    FORCEINLINE float AsFloat() const { return static_cast<float>(F64); }
    FORCEINLINE void SetInt(int32 V) { I64 = V; }
    FORCEINLINE void SetFloat(float V) { F64 = V; }
    
    // 엔티티 핸들 (ID + Generation)
    FORCEINLINE int32 GetEntityID() const { return I32[0]; }
    FORCEINLINE int32 GetGeneration() const { return I32[1]; }
    FORCEINLINE void SetEntity(int32 ID, int32 Gen) { I32[0] = ID; I32[1] = Gen; }
    
    FHktRegister() : Raw(0) {}
    explicit FHktRegister(int64 V) : I64(V) {}
    explicit FHktRegister(double V) : F64(V) {}
};

static_assert(sizeof(FHktRegister) == 8, "FHktRegister must be 8 bytes");

// ============================================================================
// 고정 크기 명령어 (8바이트)
// 
// DOD: 고정 크기로 인덱스 계산 O(1), 캐시 친화적
// ============================================================================

struct FHktInstruction
{
    uint8 Op;               // EHktOp
    uint8 Flags;            // 수정자 플래그
    uint8 A;                // 피연산자 A (주로 대상 레지스터)
    uint8 B;                // 피연산자 B
    uint8 C;                // 피연산자 C
    uint8 D;                // 피연산자 D (태그 인덱스 등)
    int16 Offset;           // 점프 오프셋 또는 상수 인덱스
    
    FORCEINLINE EHktOp GetOp() const { return static_cast<EHktOp>(Op); }
    FORCEINLINE void SetOp(EHktOp InOp) { Op = static_cast<uint8>(InOp); }
};

static_assert(sizeof(FHktInstruction) == 8, "FHktInstruction must be 8 bytes");

// ============================================================================
// 상수 타입
// ============================================================================

enum class EHktConstType : uint8
{
    Int32,
    Float,
    Vector,                 // FVector (12 bytes, padded to 16)
    Quat,                   // FQuat (16 bytes)
};

// 상수 엔트리 (16바이트 정렬)
struct alignas(16) FHktConstant
{
    union
    {
        int32   IntVal;
        float   FloatVal;
        float   VecVal[4];  // XYZ + padding 또는 XYZW (Quat)
    };
    EHktConstType Type;
    uint8 Padding[3];
    
    FHktConstant() : Type(EHktConstType::Int32) { FMemory::Memzero(VecVal, sizeof(VecVal)); }
    
    static FHktConstant MakeInt(int32 V) 
    { 
        FHktConstant C; 
        C.Type = EHktConstType::Int32; 
        C.IntVal = V; 
        return C; 
    }
    
    static FHktConstant MakeFloat(float V) 
    { 
        FHktConstant C; 
        C.Type = EHktConstType::Float; 
        C.FloatVal = V; 
        return C; 
    }
    
    static FHktConstant MakeVector(const FVector& V) 
    { 
        FHktConstant C; 
        C.Type = EHktConstType::Vector; 
        C.VecVal[0] = static_cast<float>(V.X); 
        C.VecVal[1] = static_cast<float>(V.Y); 
        C.VecVal[2] = static_cast<float>(V.Z); 
        C.VecVal[3] = 0.0f;
        return C; 
    }
};

static_assert(sizeof(FHktConstant) == 16, "FHktConstant must be 16 bytes");

// ============================================================================
// 속성 타입 (영구적 데이터)
// ============================================================================

enum class EHktAttrType : uint8
{
    Health,
    MaxHealth,
    Mana,
    MaxMana,
    Speed,
    Position,
    Rotation,
    // 확장 가능
    MAX
};
