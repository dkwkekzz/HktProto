#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HktCoreTypes.h"

// ============================================================================
// 기본 식별자 타입
// ============================================================================

/** 레거시 별칭 (HktCore 내부 호환용) */
using EntityId = FHktEntityId;

/** VM 핸들 (RuntimePool 내 슬롯 인덱스 + Generation) */
struct FHktVMHandle
{
    uint32 Index : 24;
    uint32 Generation : 8;
    
    static constexpr FHktVMHandle Invalid() { return {0xFFFFFF, 0}; }
    bool IsValid() const { return Index != 0xFFFFFF; }
    
    bool operator==(const FHktVMHandle& Other) const 
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

    int32 GetSignedImm12() const
    {
        // 12-bit sign extension
        int32 Val = Imm12;
        if (Val & 0x800)
        {
            Val |= 0xFFFFF000;
        }
        return Val;
    }

    int32 GetSignedImm20() const
    {
        // 20-bit sign extension
        int32 Val = Imm20;
        if (Val & 0x80000)
        {
            Val |= 0xFFF00000;
        }
        return Val;
    }
};

static_assert(sizeof(FInstruction) == 4, "Instruction must be 32 bits");

/**
 * PropertyId - 엔티티 속성 ID 상수
 * 
 * SOA 레이아웃에서 각 속성의 인덱스를 정의
 */
namespace PropertyId
{
    // === 위치/이동 ===
    constexpr uint16 PosX = 0;
    constexpr uint16 PosY = 1;
    constexpr uint16 PosZ = 2;
    constexpr uint16 RotYaw = 3;
    
    constexpr uint16 MoveTargetX = 4;
    constexpr uint16 MoveTargetY = 5;
    constexpr uint16 MoveTargetZ = 6;
    constexpr uint16 MoveSpeed = 7;
    constexpr uint16 IsMoving = 8;
    
    // === 전투/상태 ===
    constexpr uint16 Health = 10;
    constexpr uint16 MaxHealth = 11;
    constexpr uint16 AttackPower = 12;
    constexpr uint16 Defense = 13;
    constexpr uint16 Team = 14;
    
    // === 소유/타입 ===
    constexpr uint16 OwnerEntity = 20;
    constexpr uint16 EntityType = 21;
    
    // === 이벤트 파라미터 (IntentEvent에서 전달) ===
    constexpr uint16 TargetPosX = 30;
    constexpr uint16 TargetPosY = 31;
    constexpr uint16 TargetPosZ = 32;
    constexpr uint16 Param0 = 33;
    constexpr uint16 Param1 = 34;
    constexpr uint16 Param2 = 35;
    constexpr uint16 Param3 = 36;
    
    // === 애니메이션/비주얼 ===
    constexpr uint16 AnimState = 40;
    constexpr uint16 VisualState = 41;
}

/**
 * EntityType - 엔티티 종류 상수
 */
namespace EntityType
{
    constexpr int32 None = 0;
    constexpr int32 Unit = 1;
    constexpr int32 Projectile = 2;
    constexpr int32 Equipment = 3;
    constexpr int32 Building = 4;
}
