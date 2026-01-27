#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

// ============================================================================
// 기본 식별자 타입
// ============================================================================

/** 엔티티 식별자 (Stash 내 엔티티 인덱스) */
using EntityId = uint32;
constexpr EntityId InvalidEntityId = INDEX_NONE;

/** VM 핸들 (RuntimePool 내 슬롯 인덱스 + Generation) */
struct FVMHandle
{
    uint32 Index : 24;
    uint32 Generation : 8;
    
    static constexpr FVMHandle Invalid() { return {0xFFFFFF, 0}; }
    bool IsValid() const { return Index != 0xFFFFFF; }
    
    bool operator==(const FVMHandle& Other) const 
    { 
        return Index == Other.Index && Generation == Other.Generation; 
    }
};

/** 레지스터 인덱스 */
using RegisterIndex = uint8;
constexpr int32 MaxRegisters = 16;

// ============================================================================
// VM 상태
// ============================================================================

enum class EVMStatus : uint8
{
    Ready,          // 실행 대기
    Running,        // 실행 중
    Yielded,        // yield 상태 (다음 틱에 재개)
    Completed,      // 정상 완료
    Failed,         // 오류로 중단
};

// ============================================================================
// IntentEvent - 플레이어/AI 의도를 나타내는 입력 이벤트
// ============================================================================

struct FIntentEvent
{
    /** 이벤트 발생 프레임 (결정론적 순서 보장) */
    int32 FrameNumber = 0;
    
    /** 이벤트를 발생시킨 엔티티 */
    EntityId SourceEntity = InvalidEntityId;
    
    /** 실행할 Flow를 결정하는 태그 (예: Ability.Skill.Fireball) */
    FGameplayTag EventTag;
    
    /** 타겟 엔티티 (선택적) */
    EntityId TargetEntity = InvalidEntityId;
    
    /** 타겟 위치 (선택적) */
    FVector TargetLocation = FVector::ZeroVector;
    
    /** 추가 파라미터 (Flow별 커스텀 데이터) */
    TArray<int32> Parameters;
    
    /** 정렬을 위한 비교 (프레임 → 엔티티 → 태그) */
    bool operator<(const FIntentEvent& Other) const
    {
        if (FrameNumber != Other.FrameNumber) return FrameNumber < Other.FrameNumber;
        if (SourceEntity != Other.SourceEntity) return SourceEntity < Other.SourceEntity;
        return EventTag.GetTagName().LexicalLess(Other.EventTag.GetTagName());
    }
};

// ============================================================================
// OpCode 정의
// ============================================================================

enum class EOpCode : uint8
{
    // Control Flow
    Nop = 0,
    Halt,           // 프로그램 종료
    Yield,          // 다음 틱까지 대기
    Jump,           // 무조건 점프
    JumpIf,         // 조건부 점프
    JumpIfNot,      // 조건부 점프 (반전)
    
    // Data Operations
    LoadConst,      // 상수 → 레지스터
    LoadStore,      // Store 속성 → 레지스터
    SaveStore,      // 레지스터 → Store 속성 (버퍼링됨)
    Move,           // 레지스터 → 레지스터
    
    // Arithmetic
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    
    // Comparison (결과를 플래그 레지스터에 저장)
    CmpEq,
    CmpNe,
    CmpLt,
    CmpLe,
    CmpGt,
    CmpGe,
    
    // Game Actions (실제 게임 효과 발생)
    PlayAnim,       // 애니메이션 재생
    SpawnActor,     // 액터 스폰
    ApplyDamage,    // 데미지 적용
    ApplyEffect,    // 이펙트 적용
    PlaySound,      // 사운드 재생
    
    // Utility
    Log,            // 디버그 로그
    
    Max
};

// ============================================================================
// 명령어 인코딩
// ============================================================================

/**
 * 32비트 명령어 포맷:
 * [OpCode:8][Dst:4][Src1:4][Src2:4][Imm12:12] - 3-operand
 * [OpCode:8][Dst:4][Imm20:20]                 - Load immediate
 */
struct FInstruction
{
    union
    {
        uint32 Raw;
        struct
        {
            uint32 OpCode : 8;
            uint32 Dst : 4;
            uint32 Src1 : 4;
            uint32 Src2 : 4;
            uint32 Imm12 : 12;
        };
        struct
        {
            uint32 _Op : 8;
            uint32 _Dst : 4;
            uint32 Imm20 : 20;
        };
    };
    
    FInstruction() : Raw(0) {}
    explicit FInstruction(uint32 InRaw) : Raw(InRaw) {}
    
    EOpCode GetOpCode() const { return static_cast<EOpCode>(OpCode); }
    
    // 빌더 헬퍼
    static FInstruction Make(EOpCode Op, uint8 Dst = 0, uint8 Src1 = 0, uint8 Src2 = 0, uint16 Imm = 0)
    {
        FInstruction I;
        I.OpCode = static_cast<uint8>(Op);
        I.Dst = Dst;
        I.Src1 = Src1;
        I.Src2 = Src2;
        I.Imm12 = Imm;
        return I;
    }
    
    static FInstruction MakeImm(EOpCode Op, uint8 Dst, int32 Imm)
    {
        FInstruction I;
        I.OpCode = static_cast<uint8>(Op);
        I._Dst = Dst;
        I.Imm20 = static_cast<uint32>(Imm) & 0xFFFFF;
        return I;
    }
};

static_assert(sizeof(FInstruction) == 4, "Instruction must be 32 bits");