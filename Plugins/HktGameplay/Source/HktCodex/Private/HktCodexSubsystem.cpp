#include "HktCodexSubsystem.h"
#include "HktServiceSubsystem.h"
#include "Engine/AssetManager.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "HktActionDataAsset.h" // 인덱싱 대상 클래스 (필요 시 UDataAsset 등으로 변경 가능)

void UHktCodexSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // 인덱스 구축
    BuildAssetIndex();
}

void UHktCodexSubsystem::Deinitialize()
{
    AssetIndex.Empty();
    bIsIndexBuilt = false;
    Super::Deinitialize();
}

void UHktCodexSubsystem::BuildAssetIndex()
{
    if (bIsIndexBuilt)
    {
        return;
    }

    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    TArray<FAssetData> AssetDataList;
    FTopLevelAssetPath ClassPath = FT_UHktActionDataAsset::StaticClass()->GetClassPathName();
    AssetRegistry.GetAssetsByClass(ClassPath, AssetDataList, true);

    const FName TagName = TEXT("ActionTag");

    for (const FAssetData& Data : AssetDataList)
    {
        if (Data.TagsAndValues.Contains(TagName))
        {
            FString TagString = Data.TagsAndValues[TagName];
            FGameplayTag ActionTag = FGameplayTag::RequestGameplayTag(FName(*TagString));

            if (ActionTag.IsValid())
            {
                // TMap 사용으로 중복된 키가 있을 경우 덮어쓰거나, 로그를 남겨 중복을 경고할 수 있습니다.
                if (AssetIndex.Contains(ActionTag))
                {
                    UE_LOG(LogTemp, Warning, TEXT("Duplicate Asset Tag Detected: %s. Overwriting."), *ActionTag.ToString());
                }
                AssetIndex.Add(ActionTag, Data.ToSoftObjectPath());
            }
        }
    }

    bIsIndexBuilt = true;
}

void UHktCodexSubsystem::QueryDataAssetByTag(const FGameplayTag& InTag, FOnQueryDataComplete Callback) const
{
    // 1. 유효하지 않은 태그 처리
    if (!InTag.IsValid())
    {
        Callback.ExecuteIfBound(nullptr);
        return;
    }

    // 2. 인덱스 검색 (TMap Find)
    const FSoftObjectPath* FoundPath = AssetIndex.Find(InTag);
    if (!FoundPath)
    {
        // 태그에 매핑된 에셋이 없음
        Callback.ExecuteIfBound(nullptr);
        return;
    }

    // 3. 비동기 로딩 요청 (단일 에셋)
    FStreamableManager& Streamable = UAssetManager::Get().GetStreamableManager();
    
    // FSoftObjectPath는 포인터가 아닌 값으로 복사하여 람다에 전달
    FSoftObjectPath PathToLoad = *FoundPath;

    Streamable.RequestAsyncLoad(
        PathToLoad,
        FStreamableDelegate::CreateLambda([Callback, PathToLoad]()
        {
            UDataAsset* LoadedAsset = nullptr;
            
            if (UObject* ResolvedObj = PathToLoad.ResolveObject())
            {
                LoadedAsset = Cast<UDataAsset>(ResolvedObj);
            }

            Callback.ExecuteIfBound(LoadedAsset);
        })
    );
}