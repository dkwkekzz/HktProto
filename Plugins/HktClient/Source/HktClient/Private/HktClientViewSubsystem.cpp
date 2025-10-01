#include "HktClientViewSubsystem.h"
#include "HktClientCoreSubsystem.h"
#include "HktDataAsset.h"
#include "HktViewHandle.h"
#include "HktBehavior.h"
#include "Engine/AssetManager.h"

void UHktClientViewSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // 게임 인스턴스로부터 코어 서브시스템을 가져와 델리게이트에 핸들러를 바인딩합니다.
    if (UHktClientCoreSubsystem* Core = UHktClientCoreSubsystem::Get(GetWorld()))
    {
        Core->OnBehaviorCreated.AddUObject(this, &UHktClientViewSubsystem::HandleBehaviorCreated);
        Core->OnBehaviorDestroyed.AddUObject(this, &UHktClientViewSubsystem::HandleBehaviorDestroyed);
        Core->OnSubjectDestroyed.AddUObject(this, &UHktClientViewSubsystem::HandleSubjectDestroyed);
    }
}

void UHktClientViewSubsystem::Deinitialize()
{
    // 서브시스템이 종료될 때, 관리하던 모든 뷰가 월드에 남아있지 않도록 정리합니다.
    for (auto const& SubjectPair : LookupObjToView)
    {
        for (auto const& BehaviorPair : SubjectPair.Value)
        {
            for (UHktViewHandle* Handle : BehaviorPair.Value)
            {
                if (IsValid(Handle))
                {
                    Handle->DestroyView();
                }
            }
        }
    }
    LookupObjToView.Empty();
    LookupViewToObj.Empty();

    // 델리게이트 바인딩을 해제합니다.
    if (UHktClientCoreSubsystem* Core = UHktClientCoreSubsystem::Get(GetWorld()))
    {
        Core->OnBehaviorCreated.RemoveAll(this);
        Core->OnBehaviorDestroyed.RemoveAll(this);
        Core->OnSubjectDestroyed.RemoveAll(this);
    }

    Super::Deinitialize();
}

void UHktClientViewSubsystem::HandleBehaviorCreated(const IHktBehavior& InBehavior)
{
    const FHktId SubjectId = InBehavior.GetSubjectId();
    const FHktId BehaviorId = InBehavior.GetBehaviorId();
    const FPrimaryAssetId AssetId = InBehavior.GetViewAssetId();

    UAssetManager& AssetManager = UAssetManager::Get();

    UHktDataAsset* DataAsset = Cast<UHktDataAsset>(AssetManager.GetPrimaryAssetObject(AssetId));
    if (!DataAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("HandleBehaviorCreated: Failed to load HktDataAsset with ID: %s"), *AssetId.ToString());
        return;
    }

    // 데이터 에셋의 가상 함수를 호출하여 뷰 핸들을 생성합니다.
    UHktViewHandle* NewHandle = DataAsset->CreateViewHandle(this, FTransform::Identity); // TODO: Transform을 Behavior에서 가져와야 할 수 있음
    if (NewHandle)
    {
        // 룩업 맵들에 새 핸들을 등록합니다.
        LookupObjToView.FindOrAdd(SubjectId).FindOrAdd(BehaviorId).Add(NewHandle);
        LookupViewToObj.Add(NewHandle, MakeTuple(SubjectId, BehaviorId));

        UE_LOG(LogTemp, Log, TEXT("View created for Subject: %llu, Behavior: %llu"), SubjectId, BehaviorId);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create view from DataAsset '%s'."), *DataAsset->GetName());
    }
}

void UHktClientViewSubsystem::HandleBehaviorDestroyed(const IHktBehavior& InBehavior)
{
    const FHktId SubjectId = InBehavior.GetSubjectId();
    const FHktId BehaviorId = InBehavior.GetBehaviorId();

    if (TMap<FHktId, TArray<TObjectPtr<UHktViewHandle>>>* BehaviorMap = LookupObjToView.Find(SubjectId))
    {
        if (TArray<TObjectPtr<UHktViewHandle>>* Handles = BehaviorMap->Find(BehaviorId))
        {
            // 배열을 복사하여 순회합니다. DestroyViewInternal이 원본 배열을 수정하기 때문입니다.
            TArray<TObjectPtr<UHktViewHandle>> HandlesToDestroy = *Handles;
            for (UHktViewHandle* Handle : HandlesToDestroy)
            {
                DestroyViewInternal(Handle);
            }

            // Behavior 맵에서 이 Behavior의 항목을 제거합니다.
            BehaviorMap->Remove(BehaviorId);
            UE_LOG(LogTemp, Log, TEXT("All views destroyed for Behavior: %llu"), BehaviorId);
        }
    }
}

void UHktClientViewSubsystem::HandleSubjectDestroyed(FHktId SubjectId)
{
    if (TMap<FHktId, TArray<TObjectPtr<UHktViewHandle>>>* BehaviorMap = LookupObjToView.Find(SubjectId))
    {
        // 모든 Behavior의 모든 핸들을 파괴해야 하므로, 전체를 순회합니다.
        TArray<UHktViewHandle*> AllHandlesToDestroy;
        for (auto& BehaviorPair : *BehaviorMap)
        {
            AllHandlesToDestroy.Append(BehaviorPair.Value);
        }

        for (UHktViewHandle* Handle : AllHandlesToDestroy)
        {
            DestroyViewInternal(Handle);
        }

        // Subject 맵에서 이 Subject의 항목을 완전히 제거합니다.
        LookupObjToView.Remove(SubjectId);
        UE_LOG(LogTemp, Log, TEXT("All views destroyed for Subject: %llu"), SubjectId);
    }
}

void UHktClientViewSubsystem::DestroyViewInternal(UHktViewHandle* ViewHandle)
{
    if (!IsValid(ViewHandle))
    {
        return;
    }

    // 역방향 룩업 맵에서 ID 정보를 찾고, 해당 항목을 제거합니다.
    if (const TTuple<FHktId, FHktId>* Ids = LookupViewToObj.Find(ViewHandle))
    {
        const FHktId SubjectId = Ids->Get<0>();
        const FHktId BehaviorId = Ids->Get<1>();

        // 정방향 룩업 맵에서도 핸들을 제거합니다.
        if (TMap<FHktId, TArray<TObjectPtr<UHktViewHandle>>>* BehaviorMap = LookupObjToView.Find(SubjectId))
        {
            if (TArray<TObjectPtr<UHktViewHandle>>* Handles = BehaviorMap->Find(BehaviorId))
            {
                Handles->Remove(ViewHandle);
            }
        }

        LookupViewToObj.Remove(ViewHandle);
    }

    // 핸들을 통해 실제 뷰 리소스를 파괴합니다.
    ViewHandle->DestroyView();
}

