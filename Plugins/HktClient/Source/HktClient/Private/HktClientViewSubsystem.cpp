#include "HktClientViewSubsystem.h"
#include "HktClientCoreSubsystem.h"
#include "HktDataAsset.h"
#include "HktViewHandle.h"
#include "HktBehavior.h"
#include "Engine/AssetManager.h"

void UHktClientViewSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // ���� �ν��Ͻ��κ��� �ھ� ����ý����� ������ ��������Ʈ�� �ڵ鷯�� ���ε��մϴ�.
    if (UHktClientCoreSubsystem* Core = UHktClientCoreSubsystem::Get(GetWorld()))
    {
        Core->OnBehaviorCreated.AddUObject(this, &UHktClientViewSubsystem::HandleBehaviorCreated);
        Core->OnBehaviorDestroyed.AddUObject(this, &UHktClientViewSubsystem::HandleBehaviorDestroyed);
        Core->OnSubjectDestroyed.AddUObject(this, &UHktClientViewSubsystem::HandleSubjectDestroyed);
    }
}

void UHktClientViewSubsystem::Deinitialize()
{
    // ����ý����� ����� ��, �����ϴ� ��� �䰡 ���忡 �������� �ʵ��� �����մϴ�.
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

    // ��������Ʈ ���ε��� �����մϴ�.
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

    // ������ ������ ���� �Լ��� ȣ���Ͽ� �� �ڵ��� �����մϴ�.
    UHktViewHandle* NewHandle = DataAsset->CreateViewHandle(this, FTransform::Identity); // TODO: Transform�� Behavior���� �����;� �� �� ����
    if (NewHandle)
    {
        // ��� �ʵ鿡 �� �ڵ��� ����մϴ�.
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
            // �迭�� �����Ͽ� ��ȸ�մϴ�. DestroyViewInternal�� ���� �迭�� �����ϱ� �����Դϴ�.
            TArray<TObjectPtr<UHktViewHandle>> HandlesToDestroy = *Handles;
            for (UHktViewHandle* Handle : HandlesToDestroy)
            {
                DestroyViewInternal(Handle);
            }

            // Behavior �ʿ��� �� Behavior�� �׸��� �����մϴ�.
            BehaviorMap->Remove(BehaviorId);
            UE_LOG(LogTemp, Log, TEXT("All views destroyed for Behavior: %llu"), BehaviorId);
        }
    }
}

void UHktClientViewSubsystem::HandleSubjectDestroyed(FHktId SubjectId)
{
    if (TMap<FHktId, TArray<TObjectPtr<UHktViewHandle>>>* BehaviorMap = LookupObjToView.Find(SubjectId))
    {
        // ��� Behavior�� ��� �ڵ��� �ı��ؾ� �ϹǷ�, ��ü�� ��ȸ�մϴ�.
        TArray<UHktViewHandle*> AllHandlesToDestroy;
        for (auto& BehaviorPair : *BehaviorMap)
        {
            AllHandlesToDestroy.Append(BehaviorPair.Value);
        }

        for (UHktViewHandle* Handle : AllHandlesToDestroy)
        {
            DestroyViewInternal(Handle);
        }

        // Subject �ʿ��� �� Subject�� �׸��� ������ �����մϴ�.
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

    // ������ ��� �ʿ��� ID ������ ã��, �ش� �׸��� �����մϴ�.
    if (const TTuple<FHktId, FHktId>* Ids = LookupViewToObj.Find(ViewHandle))
    {
        const FHktId SubjectId = Ids->Get<0>();
        const FHktId BehaviorId = Ids->Get<1>();

        // ������ ��� �ʿ����� �ڵ��� �����մϴ�.
        if (TMap<FHktId, TArray<TObjectPtr<UHktViewHandle>>>* BehaviorMap = LookupObjToView.Find(SubjectId))
        {
            if (TArray<TObjectPtr<UHktViewHandle>>* Handles = BehaviorMap->Find(BehaviorId))
            {
                Handles->Remove(ViewHandle);
            }
        }

        LookupViewToObj.Remove(ViewHandle);
    }

    // �ڵ��� ���� ���� �� ���ҽ��� �ı��մϴ�.
    ViewHandle->DestroyView();
}

