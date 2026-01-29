// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HktCoreTypes.h"
#include "HktCoreInterfaces.generated.h"

//=============================================================================
// IStashInterface - 순수 C++ Stash 인터페이스
//=============================================================================

/**
 * IStashInterface - Stash 공통 인터페이스 (Pure C++)
 * 
 * HktCore는 순수 모듈이므로 UObject 참조 없이 인터페이스만 정의
 * 실제 구현은 HktRuntime의 컴포넌트에서 제공
 */
class HKTCORE_API IStashInterface
{
public:
    virtual ~IStashInterface() = default;
    
    virtual bool IsValidEntity(FHktEntityId Entity) const = 0;
    virtual int32 GetProperty(FHktEntityId Entity, uint16 PropertyId) const = 0;
    virtual void SetProperty(FHktEntityId Entity, uint16 PropertyId, int32 Value) = 0;
    virtual FHktEntityId AllocateEntity() = 0;
    virtual void FreeEntity(FHktEntityId Entity) = 0;
};
