#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HktVMTypes.h"

// ============================================================================
// HktVM Program - 불변 바이트코드 프로그램
// 
// 구조:
// - Instructions: 고정 크기 명령어 배열 (8바이트 정렬)
// - TagTable: GameplayTag 룩업 테이블 (인덱스로 참조)
// - ConstPool: 상수 풀 (float, vector 등)
// 
// Flow 정의에서 빌드되어 캐시됨
// ============================================================================

struct FHktProgram
{
    // 명령어 배열 (고정 크기, 8바이트 정렬)
    TArray<FHktInstruction> Instructions;
    
    // 태그 테이블 - 명령어에서 8비트 인덱스로 참조
    // PlayAnimation(TagIndex=5) -> TagTable[5] = "Anim.Mage.FireballCast"
    TArray<FGameplayTag> TagTable;
    
    // 상수 풀 - 명령어에서 16비트 인덱스로 참조
    TArray<FHktConstant> ConstPool;
    
    // 프로그램 메타데이터
    FGameplayTag FlowTag;           // 이 프로그램이 처리하는 이벤트 태그
    int32 RequiredRegisters = 0;    // 필요한 레지스터 수
    
    // ========================================================================
    // 태그 테이블 접근
    // ========================================================================
    
    FORCEINLINE const FGameplayTag& GetTag(uint8 Index) const
    {
        checkf(Index < TagTable.Num(), TEXT("Tag index out of bounds: %d"), Index);
        return TagTable[Index];
    }
    
    uint8 AddTag(const FGameplayTag& Tag)
    {
        // 중복 체크
        int32 Existing = TagTable.Find(Tag);
        if (Existing != INDEX_NONE)
        {
            return static_cast<uint8>(Existing);
        }
        
        checkf(TagTable.Num() < HKT_MAX_TAGS, TEXT("Tag table overflow"));
        int32 Index = TagTable.Add(Tag);
        return static_cast<uint8>(Index);
    }
    
    // ========================================================================
    // 상수 풀 접근
    // ========================================================================
    
    FORCEINLINE const FHktConstant& GetConst(int16 Index) const
    {
        checkf(Index >= 0 && Index < ConstPool.Num(), TEXT("Const index out of bounds: %d"), Index);
        return ConstPool[Index];
    }
    
    int16 AddConstInt(int32 Value)
    {
        int32 Index = ConstPool.Add(FHktConstant::MakeInt(Value));
        checkf(Index < HKT_MAX_CONSTANTS, TEXT("Constant pool overflow"));
        return static_cast<int16>(Index);
    }
    
    int16 AddConstFloat(float Value)
    {
        int32 Index = ConstPool.Add(FHktConstant::MakeFloat(Value));
        checkf(Index < HKT_MAX_CONSTANTS, TEXT("Constant pool overflow"));
        return static_cast<int16>(Index);
    }
    
    int16 AddConstVector(const FVector& Value)
    {
        int32 Index = ConstPool.Add(FHktConstant::MakeVector(Value));
        checkf(Index < HKT_MAX_CONSTANTS, TEXT("Constant pool overflow"));
        return static_cast<int16>(Index);
    }
    
    // ========================================================================
    // 명령어 접근
    // ========================================================================
    
    FORCEINLINE const FHktInstruction& GetInstruction(int32 PC) const
    {
        checkf(PC >= 0 && PC < Instructions.Num(), TEXT("PC out of bounds: %d"), PC);
        return Instructions[PC];
    }
    
    FORCEINLINE int32 GetInstructionCount() const
    {
        return Instructions.Num();
    }
    
    // ========================================================================
    // 유효성 검사
    // ========================================================================
    
    bool IsValid() const
    {
        return Instructions.Num() > 0;
    }
    
    // 디버그 출력
    void DebugDump() const
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(LogTemp, Log, TEXT("=== HktProgram: %s ==="), *FlowTag.ToString());
        UE_LOG(LogTemp, Log, TEXT("Instructions: %d, Tags: %d, Constants: %d"),
            Instructions.Num(), TagTable.Num(), ConstPool.Num());
        
        for (int32 i = 0; i < Instructions.Num(); ++i)
        {
            const FHktInstruction& Inst = Instructions[i];
            UE_LOG(LogTemp, Log, TEXT("[%04d] Op=%d A=%d B=%d C=%d D=%d Off=%d"),
                i, Inst.Op, Inst.A, Inst.B, Inst.C, Inst.D, Inst.Offset);
        }
#endif
    }
};

// ============================================================================
// 프로그램 캐시
// GameplayTag -> FHktProgram 매핑
// ============================================================================

class FHktProgramCache
{
public:
    // 프로그램 가져오기 (없으면 nullptr)
    const FHktProgram* Get(const FGameplayTag& FlowTag) const
    {
        const TSharedPtr<FHktProgram>* Found = Cache.Find(FlowTag);
        return Found ? Found->Get() : nullptr;
    }
    
    // 프로그램 추가
    void Add(const FGameplayTag& FlowTag, TSharedPtr<FHktProgram> Program)
    {
        Cache.Add(FlowTag, Program);
    }
    
    // 캐시 클리어
    void Clear()
    {
        Cache.Empty();
    }
    
    // 캐시된 프로그램 수
    int32 Num() const
    {
        return Cache.Num();
    }
    
private:
    TMap<FGameplayTag, TSharedPtr<FHktProgram>> Cache;
};
