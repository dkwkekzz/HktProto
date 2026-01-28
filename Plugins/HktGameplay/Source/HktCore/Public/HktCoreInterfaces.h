// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HktCoreTypes.h"
#include "HktCoreInterfaces.generated.h"

//=============================================================================
// IHktSelectable - 선택 가능한 오브젝트 인터페이스
//=============================================================================

UINTERFACE(MinimalAPI, BlueprintType)
class UHktSelectable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 선택 가능한 오브젝트가 구현하는 인터페이스.
 * EntityId를 제공하여 Intent 시스템에서 Subject/Target으로 사용됨.
 */
class HKCORE_API IHktSelectable
{
	GENERATED_BODY()

public:
	/** 이 오브젝트의 EntityId를 반환 */
	virtual FHktEntityId GetEntityId() const = 0;
	
	/** 선택 가능 여부 반환 (기본: true) */
	virtual bool IsSelectable() const { return true; }
};

//=============================================================================
// IHktIntentEventProvider - Intent 이벤트 제공자 인터페이스
//=============================================================================

UINTERFACE(MinimalAPI, Blueprintable)
class UHktIntentEventProvider : public UInterface
{
	GENERATED_BODY()
};

class HKCORE_API IHktIntentEventProvider
{
	GENERATED_BODY()

public:
	virtual void PullIntentEvents(int32 CompletedFrameNumber, TArray<FHktIntentEvent>& OutIntentEvents) = 0;
};