#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Delegates/Delegate.h"
#include "HAL/RunnableThread.h"

// Unreal Engine�� FRunnable�� ���� �Լ��� �Բ� ����ϱ� ���� ���� Ŭ�����Դϴ�.
class FFunctionRunnable : public FRunnable
{
public:
    // ������ ���� �Լ��� �޴� ������
    FFunctionRunnable(TFunction<void()> InFunction) : Function(MoveTemp(InFunction)) {}

    // FRunnable �������̽� ����
    virtual uint32 Run() override
    {
        if (Function)
        {
            Function(); // ����� ���� �Լ� ����
        }
        return 0;
    }

private:
    TFunction<void()> Function;
};

// HKT �ý��ۿ��� ����ϴ� ID Ÿ�԰� ��ȿ���� ���� ID ��� ����
using FHktId = int64;
constexpr FHktId InvalidHktId = -1;
