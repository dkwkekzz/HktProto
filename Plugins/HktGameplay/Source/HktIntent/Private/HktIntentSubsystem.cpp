#include "HktIntentSubsystem.h"
#include "HktIntentComponent.h"
#include "HktIntentEffectMappingAsset.h"
#include "HktServiceSubsystem.h"
#include "Engine/AssetManager.h"

static const TArray<FHktIntentEffect> EmptyEffects;

void UHktIntentSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // 매핑 어셋 로드
    LoadEffectMappingAssets();
    
    if (UHktServiceSubsystem* Service = Collection.InitializeDependency<UHktServiceSubsystem>())
    {
        Service->RegisterIntentEventProvider(this);
    }
}

void UHktIntentSubsystem::Deinitialize()
{
    OwnerEffectMap.Empty();
    EffectMappingRegistry.Empty();
    CachedEffectOwners.Empty();

    if (UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld()))
    {
        Service->UnregisterIntentEventProvider(this);
    }

    Super::Deinitialize();
}

void UHktIntentSubsystem::ProcessIntentChange(const FHktIntentEvent& Event, EIntentChangeType ChangeType)
{
    // Event로부터 Effect 생성/적용
    ApplyEffectsFromEvent(Event, ChangeType);
}

// ============================================================================
// IHktIntentEventProvider Implementation
// ============================================================================

const TArray<FHktUnitHandle>& UHktIntentSubsystem::GetIntentEffectOwners() const
{
    if (bEffectOwnersCacheDirty)
    {
        CachedEffectOwners.Reset();
        OwnerEffectMap.GetKeys(CachedEffectOwners);
        bEffectOwnersCacheDirty = false;
    }
    return CachedEffectOwners;
}

const TArray<FHktIntentEffect>& UHktIntentSubsystem::GetIntentEffectsForOwner(const FHktUnitHandle& OwnerHandle) const
{
    if (const TArray<FHktIntentEffect>* FoundEffects = OwnerEffectMap.Find(OwnerHandle))
    {
        return *FoundEffects;
    }
    return EmptyEffects;
}

bool UHktIntentSubsystem::HasIntentEffectWithTag(const FHktUnitHandle& OwnerHandle, FGameplayTag Tag) const
{
    const TArray<FHktIntentEffect>& Effects = GetIntentEffectsForOwner(OwnerHandle);
    return Effects.ContainsByPredicate([&](const FHktIntentEffect& E) { return E.EffectTag == Tag; });
}

// ============================================================================
// Effect Management API
// ============================================================================

void UHktIntentSubsystem::AddEffectToOwner(const FHktUnitHandle& OwnerHandle, const FHktIntentEffect& Effect)
{
    TArray<FHktIntentEffect>& EffectList = OwnerEffectMap.FindOrAdd(OwnerHandle);
    EffectList.Add(Effect);
    bEffectOwnersCacheDirty = true;
}

void UHktIntentSubsystem::RemoveEffectFromOwner(const FHktUnitHandle& OwnerHandle, int32 EffectId)
{
    if (TArray<FHktIntentEffect>* EffectList = OwnerEffectMap.Find(OwnerHandle))
    {
        EffectList->RemoveAll([EffectId](const FHktIntentEffect& E) { return E.EffectId == EffectId; });

        if (EffectList->IsEmpty())
        {
            OwnerEffectMap.Remove(OwnerHandle);
            bEffectOwnersCacheDirty = true;
        }
    }
}

void UHktIntentSubsystem::RemoveAllEffectsFromOwner(const FHktUnitHandle& OwnerHandle)
{
    if (OwnerEffectMap.Remove(OwnerHandle) > 0)
    {
        bEffectOwnersCacheDirty = true;
    }
}

// ============================================================================
// Effect Generation (Private)
// ============================================================================

void UHktIntentSubsystem::ApplyEffectsFromEvent(const FHktIntentEvent& Event, EIntentChangeType ChangeType)
{
    // Added 이벤트에만 Effect 생성
    // (Effect는 독립적으로 관리되므로 Event Removed 시 자동 제거하지 않음)
    if (ChangeType != EIntentChangeType::Added)
    {
        return;
    }

    // 매핑 어셋 조회
    UHktIntentEffectMappingAsset* MappingAsset = FindMappingAssetForEventTag(Event.EventTag);
    if (!MappingAsset)
    {
        return;
    }

    const int32 CurrentFrame = Event.FrameNumber;

    // Subject들에게 Effect 적용
    for (const FHktUnitHandle& SubjectHandle : Event.Subjects)
    {
        if (!SubjectHandle.IsValid())
        {
            continue;
        }

        for (const FHktEffectDefinition& EffectDef : MappingAsset->SubjectEffects)
        {
            if (!EffectDef.EffectTag.IsValid())
            {
                continue;
            }

            FHktIntentEffect NewEffect;
            NewEffect.EffectId = GenerateEffectId();
            NewEffect.FrameNumber = CurrentFrame;
            NewEffect.Owner = SubjectHandle;
            NewEffect.EffectTag = EffectDef.EffectTag;

            AddEffectToOwner(SubjectHandle, NewEffect);
        }
    }

    // Target에게 Effect 적용
    if (Event.Target.IsValid())
    {
        for (const FHktEffectDefinition& EffectDef : MappingAsset->TargetEffects)
        {
            if (!EffectDef.EffectTag.IsValid())
            {
                continue;
            }

            FHktIntentEffect NewEffect;
            NewEffect.EffectId = GenerateEffectId();
            NewEffect.FrameNumber = CurrentFrame;
            NewEffect.Owner = Event.Target;
            NewEffect.EffectTag = EffectDef.EffectTag;

            AddEffectToOwner(Event.Target, NewEffect);
        }
    }
}

UHktIntentEffectMappingAsset* UHktIntentSubsystem::FindMappingAssetForEventTag(FGameplayTag EventTag) const
{
    if (const TObjectPtr<UHktIntentEffectMappingAsset>* FoundAsset = EffectMappingRegistry.Find(EventTag))
    {
        return FoundAsset->Get();
    }
    return nullptr;
}

void UHktIntentSubsystem::LoadEffectMappingAssets()
{
    UAssetManager& AssetManager = UAssetManager::Get();
    
    TArray<FPrimaryAssetId> AssetIds;
    AssetManager.GetPrimaryAssetIdList(UHktIntentEffectMappingAsset::GetAssetType(), AssetIds);

    for (const FPrimaryAssetId& AssetId : AssetIds)
    {
        // 동기 로드 (Initialize 시점이므로 허용)
        FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(AssetId);
        if (UHktIntentEffectMappingAsset* LoadedAsset = Cast<UHktIntentEffectMappingAsset>(AssetPath.TryLoad()))
        {
            if (LoadedAsset->EventTag.IsValid())
            {
                EffectMappingRegistry.Add(LoadedAsset->EventTag, LoadedAsset);
            }
        }
    }
}

void UHktIntentSubsystem::RefreshCachedEffectOwners()
{
    CachedEffectOwners.Reset();
    OwnerEffectMap.GetKeys(CachedEffectOwners);
    bEffectOwnersCacheDirty = false;
}

int32 UHktIntentSubsystem::GenerateEffectId()
{
    return NextEffectId++;
}