#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HktDef.h"
#include "HktClientViewSubsystem.generated.h"

class UHktViewHandle;
class IHktBehavior;

/**
 * HktDataAsset을 참조하여 월드에 다양한 뷰 리소스(액터, 파티클 등)를 스폰하고 관리하는 서브시스템입니다.
 */
UCLASS()
class HKTCLIENT_API UHktClientViewSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

protected:
    // UWorldSubsystem overloads
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:
    // --- 이벤트 핸들러 ---
    void HandleBehaviorCreated(const IHktBehavior& InBehavior);
    void HandleBehaviorDestroyed(const IHktBehavior& InBehavior);
    void HandleSubjectDestroyed(FHktId SubjectId);

    /** 내부적으로 뷰를 파괴하고 룩업 맵에서 제거하는 헬퍼 함수입니다. */
    void DestroyViewInternal(UHktViewHandle* ViewHandle);

    /**
     * Subject ID -> Behavior ID -> View Handles 배열로의 룩업 맵입니다.
     * 특정 Subject나 Behavior에 속한 모든 뷰를 빠르게 찾기 위해 사용됩니다.
     */
    UPROPERTY()
    TMap<FHktId, TMap<FHktId, TArray<TObjectPtr<UHktViewHandle>>>> LookupObjToView;

    /**
     * View Handle -> {Subject ID, Behavior ID} 로의 역방향 룩업 맵입니다.
     * 특정 뷰가 어떤 객체에 속해있는지 찾거나, 개별 뷰 파괴 시 ObjToView 맵을 정리하기 위해 사용됩니다.
     * 키는 TWeakObjectPtr를 사용하여, 핸들이 다른 곳에서 파괴되었을 때 맵이 유효하지 않은 포인터를 들고 있지 않도록 합니다.
     */
    UPROPERTY()
    TMap<TWeakObjectPtr<UHktViewHandle>, TTuple<FHktId, FHktId>> LookupViewToObj;
};

