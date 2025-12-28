#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HktIntentInterfaces.h"
#include "HktIntentComponent.h"
#include "HktUnitHandle.h"
#include "HktIntentSubsystem.generated.h"

class UHktIntentEffectMappingAsset;

/**
 * [Central Logic Brain]
 * - UnitHandle 기반으로 Intent 이벤트를 관리
 * - Event → Effect 변환 및 제공
 */
UCLASS()
class HKTINTENT_API UHktIntentSubsystem : public UWorldSubsystem, public IHktIntentEventProvider
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // --- Public API ---
    /** 
     * 컴포넌트로부터 변경 사항 수신 
     * (Event 내부의 Subjects 배열을 순회하며 처리)
     */
    void ProcessIntentChange(const FHktIntentEvent& Event, EIntentChangeType ChangeType);

    // --- IHktIntentEventProvider Implementation ---
    virtual const TArray<FHktUnitHandle>& GetIntentEffectOwners() const override;
    virtual const TArray<FHktIntentEffect>& GetIntentEffectsForOwner(const FHktUnitHandle& OwnerHandle) const override;
    virtual bool HasIntentEffectWithTag(const FHktUnitHandle& OwnerHandle, FGameplayTag Tag) const override;

    // --- Effect Management API ---
    /** Effect를 특정 Owner에게 추가 */
    void AddEffectToOwner(const FHktUnitHandle& OwnerHandle, const FHktIntentEffect& Effect);

    /** 특정 Owner의 특정 EffectId를 가진 Effect 제거 */
    void RemoveEffectFromOwner(const FHktUnitHandle& OwnerHandle, int32 EffectId);

    /** 특정 Owner의 모든 Effect 제거 */
    void RemoveAllEffectsFromOwner(const FHktUnitHandle& OwnerHandle);
    
private:
    // --- Effect Generation ---
    /** Event로부터 Subject/Target에게 Effect 적용 */
    void ApplyEffectsFromEvent(const FHktIntentEvent& Event, EIntentChangeType ChangeType);

    /** EventTag에 해당하는 매핑 어셋 조회 */
    UHktIntentEffectMappingAsset* FindMappingAssetForEventTag(FGameplayTag EventTag) const;

    /** 매핑 어셋 로드 */
    void LoadEffectMappingAssets();

    /** CachedEffectOwners 목록 갱신 */
    void RefreshCachedEffectOwners();

    /** 고유한 EffectId 생성 */
    int32 GenerateEffectId();

private:
    /** Owner(Handle)별 Effect 목록 */
    TMap<FHktUnitHandle, TArray<FHktIntentEffect>> OwnerEffectMap;

    /** 활성화된 Effect Owner 목록 (캐시) */
    mutable TArray<FHktUnitHandle> CachedEffectOwners;

    /** CachedEffectOwners가 유효한지 여부 */
    mutable bool bEffectOwnersCacheDirty = true;

    /** EventTag → MappingAsset 레지스트리 */
    TMap<FGameplayTag, TObjectPtr<UHktIntentEffectMappingAsset>> EffectMappingRegistry;

    /** EffectId 생성용 카운터 */
    int32 NextEffectId = 1;
};