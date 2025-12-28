#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HktServiceTypes.h"
#include "IHktIntentEventProvider.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UHktIntentEventProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for a system that provides Intent Events to consumers (Simulation).
 * Implemented by HktIntent module.
 */
class HKTSERVICE_API IHktIntentEventProvider
{
	GENERATED_BODY()

public:
    /** 활성화된 의도 효과의 소유자 목록을 반환 */
    virtual const TArray<FHktUnitHandle>& GetIntentEffectOwners() const = 0;

    /** 특정 대상(Subject Handle)이 가지고 있는 활성화된 의도 목록을 반환 */
    virtual const TArray<FHktIntentEffect>& GetIntentEffectsForSubject(const FHktUnitHandle& SubjectHandle) const = 0;

    /** 특정 대상의 특정 태그를 가진 의도가 있는지 확인 */
    virtual bool HasIntentEffectWithTag(const FHktUnitHandle& SubjectHandle, FGameplayTag Tag) const = 0;
};
