#include "HktVMDispatch.h"

// 함수 포인터 테이블 정의
FHktOpHandler FHktVMDispatch::OpTable[static_cast<int32>(EHktOp::MAX)] = {};

// 자동 초기화
struct FHktVMDispatchInitializer
{
    FHktVMDispatchInitializer()
    {
        FHktVMDispatch::Initialize();
    }
};

static FHktVMDispatchInitializer GHktVMDispatchInitializer;
