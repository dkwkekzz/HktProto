#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Delegates/Delegate.h"
#include "HAL/RunnableThread.h"

// Unreal Engine의 FRunnable을 람다 함수와 함께 사용하기 위한 헬퍼 클래스입니다.
class FFunctionRunnable : public FRunnable
{
public:
    // 실행할 람다 함수를 받는 생성자
    FFunctionRunnable(TFunction<void()> InFunction) : Function(MoveTemp(InFunction)) {}

    // FRunnable 인터페이스 구현
    virtual uint32 Run() override
    {
        if (Function)
        {
            Function(); // 저장된 람다 함수 실행
        }
        return 0;
    }

private:
    TFunction<void()> Function;
};

// HKT 시스템에서 사용하는 ID 타입과 유효하지 않은 ID 상수 정의
using FHktId = int64;
constexpr FHktId InvalidHktId = -1;
