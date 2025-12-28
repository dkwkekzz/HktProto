// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Engine/HitResult.h"
#include "HktIntentTags.h"
#include "HktActionDataAsset.h"
#include "HktInputContexts.generated.h"

//-----------------------------------------------------------------------------
// Interfaces
//-----------------------------------------------------------------------------

/** 주체(Subject)에 대한 문맥 정보를 제공합니다. */
UINTERFACE(MinimalAPI, BlueprintType)
class UHktSubjectContext : public UInterface { GENERATED_BODY() };

class HKTINTENT_API IHktSubjectContext
{
    GENERATED_BODY()
public:
    /** 이 문맥이 가리키는 실제 유닛들을 반환합니다. */
    virtual TArray<FHktUnitHandle> ResolveSubjects() const = 0;
    
    virtual FHktUnitHandle ResolvePrimarySubject() const = 0;

    /** 유효성 검사 */
    virtual bool IsValid() const = 0;

    virtual bool IsPrimarySubject() const = 0;
};

/** 명령(Command)에 대한 문맥 정보를 제공합니다. */
UINTERFACE(MinimalAPI, BlueprintType)
class UHktCommandContext : public UInterface { GENERATED_BODY() };

class HKTINTENT_API IHktCommandContext
{
    GENERATED_BODY()
public:
    /** 수행할 액션 데이터를 반환합니다. */
    virtual FGameplayTag ResolveEventTag() const = 0;

    /** 명령이 유효한지(데이터를 찾았는지) 검사합니다. */
    virtual bool IsValid() const = 0;

    /** 명령이 대상(Target)을 필요로 하는지 반환합니다. */
    virtual bool IsRequiredTarget() const = 0;
};

/** 대상(Target)에 대한 문맥 정보를 제공합니다. */
UINTERFACE(MinimalAPI, BlueprintType)
class UHktTargetContext : public UInterface { GENERATED_BODY() };

class HKTINTENT_API IHktTargetContext
{
    GENERATED_BODY()
public:
    /** * 타겟의 위치를 반환합니다. 
     * 대상이 유닛인 경우 해당 유닛의 위치(혹은 소켓 위치)이며, 지형인 경우 클릭 지점입니다.
     * 비동기 처리를 고려하여 이 값은 Context 생성 시점에 미리 계산(Cache)되어 있어야 합니다.
     */
    virtual FVector GetTargetLocation() const = 0;

    /** * 타겟 유닛 핸들을 반환합니다. 
     * 대상이 지형이거나 유효한 유닛이 아닌 경우 Invalid Handle을 반환합니다.
     * 비동기 처리를 고려하여 이 값은 Context 생성 시점에 미리 계산(Cache)되어 있어야 합니다.
     */
    virtual FHktUnitHandle GetTargetUnit() const = 0;

    /** 타겟 데이터가 유효한지 검사합니다. */
    virtual bool IsValid() const = 0;
};


//-----------------------------------------------------------------------------
// Concrete Implementations
//-----------------------------------------------------------------------------

/** * 클릭(HitResult)을 통해 주체를 결정하는 컨텍스트 
 */
UCLASS()
class HKTINTENT_API UHktSubjectContext_ByClick : public UObject, public IHktSubjectContext
{
    GENERATED_BODY()

public:
    void Initialize(const FHitResult& InHit);

    // IHktSubjectContext Interface
    virtual TArray<FHktUnitHandle> ResolveSubjects() const override;
    virtual FHktUnitHandle ResolvePrimarySubject() const override;
    virtual bool IsValid() const override { return CachedHit.bBlockingHit; }
    virtual bool IsPrimarySubject() const override { return false; }

private:
    UPROPERTY()
    FHitResult CachedHit;
};

/** * 슬롯 인덱스를 기반으로 명령을 결정하는 컨텍스트
 * 생성 시점에 Subject와 SlotIndex를 받아 실제 ActionDataAsset을 미리 조회(Caching)합니다.
 */
UCLASS()
class HKTINTENT_API UHktCommandContext_BySlot : public UObject, public IHktCommandContext
{
    GENERATED_BODY()

public:
    /** * 초기화 시점에 Provider에 접근하여 실제 ActionAsset을 찾아냅니다. 
     * 나중에 Resolve 호출 시 지연 없이 데이터를 제공하기 위함입니다.
     */
    void Initialize(const TScriptInterface<IHktSubjectContext>& InSubjectContext, int32 InSlotIndex);

    // IHktCommandContext Interface
    virtual FGameplayTag ResolveEventTag() const override;
    virtual bool IsValid() const override;
    virtual bool IsRequiredTarget() const override;

private:
    /** 미리 조회된 액션 데이터 */
    UPROPERTY()
    TObjectPtr<UHktActionDataAsset> CachedActionAsset;
};

/** * 커서 위치(HitResult)를 통해 대상(Unit/Location)을 결정하는 컨텍스트
 * Initialize 시점에 HitResult를 분석하여 UnitHandle과 Location을 미리 캐싱합니다.
 */
UCLASS()
class HKTINTENT_API UHktTargetContext_ByClick : public UObject, public IHktTargetContext
{
    GENERATED_BODY()

public:
    /** 초기화 시점에 HitResult를 분석하여 데이터를 캐싱합니다. */
    void Initialize(const FHitResult& InHit);

    // IHktTargetContext Interface
    virtual FVector GetTargetLocation() const override;
    virtual FHktUnitHandle GetTargetUnit() const override;
    virtual bool IsValid() const override;

private:
    /** 캐싱된 위치 정보 (지면 혹은 유닛 위치) */
    UPROPERTY()
    FVector CachedLocation;

    /** 캐싱된 유닛 핸들 (없으면 Invalid) */
    UPROPERTY()
    FHktUnitHandle CachedUnitHandle;
};