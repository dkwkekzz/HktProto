// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "HktServiceTypes.h"
#include "HktIntentBuilderComponent.generated.h"

/**
 * 입력을 조립하여 Intent를 빌드하는 컴포넌트.
 * Subject, Command, Target을 순차적으로 설정하고 제출합니다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HKTINTENT_API UHktIntentBuilderComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHktIntentBuilderComponent();

    //-------------------------------------------------------------------------
    // Action Creation (입력 핸들러에서 호출)
    //-------------------------------------------------------------------------
    
    /** 주체 선택 - 커서 아래 유닛을 Subject로 설정 */
    UFUNCTION(BlueprintCallable, Category = "Hkt|IntentBuilder")
    void CreateSubjectAction();
    
    /** 명령 선택 - 슬롯 인덱스로 Command 설정 */
    UFUNCTION(BlueprintCallable, Category = "Hkt|IntentBuilder")
    void CreateCommandAction(int32 SlotIndex);
    
    /** 대상 선택 - 커서 아래를 Target으로 설정 */
    UFUNCTION(BlueprintCallable, Category = "Hkt|IntentBuilder")
    void CreateTargetAction();

    //-------------------------------------------------------------------------
    // Data Access (IntentComponent에서 호출)
    //-------------------------------------------------------------------------
    
    /** 주체 유닛 핸들 */
    UFUNCTION(BlueprintCallable, Category = "Hkt|IntentBuilder")
    FHktUnitHandle GetSubject() const { return SubjectHandle; }
    
    /** 명령 이벤트 태그 */
    UFUNCTION(BlueprintCallable, Category = "Hkt|IntentBuilder")
    FGameplayTag GetEventTag() const { return EventTag; }
    
    /** 대상 유닛 핸들 (유닛이 아니면 Invalid) */
    UFUNCTION(BlueprintCallable, Category = "Hkt|IntentBuilder")
    FHktUnitHandle GetTargetUnit() const { return TargetHandle; }
    
    /** 대상 위치 */
    UFUNCTION(BlueprintCallable, Category = "Hkt|IntentBuilder")
    FVector GetTargetLocation() const { return TargetLocation; }

    //-------------------------------------------------------------------------
    // Validation
    //-------------------------------------------------------------------------
    
    /** Subject와 Command가 유효한지 검사 */
    UFUNCTION(BlueprintCallable, Category = "Hkt|IntentBuilder")
    bool IsValid() const;
    
    /** 제출 가능한 상태인지 검사 (Target 필요 여부 포함) */
    UFUNCTION(BlueprintCallable, Category = "Hkt|IntentBuilder")
    bool IsReadyToSubmit() const;
    
    /** Target이 필요한 Command인지 */
    UFUNCTION(BlueprintCallable, Category = "Hkt|IntentBuilder")
    bool IsTargetRequired() const { return bTargetRequired; }

    //-------------------------------------------------------------------------
    // Lifecycle
    //-------------------------------------------------------------------------
    
    /** 모든 데이터 초기화 */
    UFUNCTION(BlueprintCallable, Category = "Hkt|IntentBuilder")
    void Reset();
    
    /** Command와 Target만 초기화 (Subject 유지) */
    UFUNCTION(BlueprintCallable, Category = "Hkt|IntentBuilder")
    void ResetCommand();

protected:
    /** 커서 아래 HitResult 가져오기 */
    bool GetHitUnderCursor(FHitResult& OutHit) const;
    
    /** Hit로부터 Subject 캐싱 */
    void CacheSubjectFromHit(const FHitResult& Hit);
    
    /** Hit로부터 Target 캐싱 */
    void CacheTargetFromHit(const FHitResult& Hit);

private:
    UPROPERTY(Transient)
    FHktUnitHandle SubjectHandle;
    
    UPROPERTY(Transient)
    FGameplayTag EventTag;
    
    UPROPERTY(Transient)
    FHktUnitHandle TargetHandle;
    
    UPROPERTY(Transient)
    FVector TargetLocation = FVector::ZeroVector;
    
    UPROPERTY(Transient)
    bool bTargetRequired = true;
};
