// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktInputContexts.h"
#include "HktServiceSubsystem.h"
#include "IHktSelectionProvider.h"
#include "IHktGameDataProvider.h"

//-----------------------------------------------------------------------------
// Subject Context - By Click
//-----------------------------------------------------------------------------

void UHktSubjectContext_ByClick::Initialize(const FHitResult& InHit)
{
    CachedHit = InHit;
}

TArray<FHktUnitHandle> UHktSubjectContext_ByClick::ResolveSubjects() const
{
    TArray<FHktUnitHandle> OutSubjects;

    if (!CachedHit.bBlockingHit)
    {
        return OutSubjects;
    }

    // UObject의 GetWorld()를 사용하여 월드 컨텍스트 획득 (Outer가 PlayerController라고 가정)
    if (UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld()))
    {
        if (const auto& Provider = Service->GetSelectionProvider())
        {
            Provider->QuerySelectUnits(CachedHit, OutSubjects);
        }
    }

    return OutSubjects;
}

FHktUnitHandle UHktSubjectContext_ByClick::ResolvePrimarySubject() const
{
    TArray<FHktUnitHandle> Subjects = ResolveSubjects();
    return Subjects.Num() > 0 ? Subjects[0] : FHktUnitHandle();
}

//-----------------------------------------------------------------------------
// Command Context - By Slot
//-----------------------------------------------------------------------------

void UHktCommandContext_BySlot::Initialize(const TScriptInterface<IHktSubjectContext>& InSubjectContext, int32 InSlotIndex)
{
    CachedActionAsset = nullptr;

    if (!InSubjectContext)
    {
        return;
    }

    // 1. 주체 핸들 조회
    const FHktUnitHandle& PrimarySubject = InSubjectContext->ResolvePrimarySubject();
    if (!PrimarySubject.IsValid())
    {
        return;
    }

    // 2. 비동기 데이터 요청 (Async Evaluation)
    if (UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld()))
    {
        if (const auto& Provider = Service->GetCodexProvider())
        {
            // 콜백 델리게이트 생성 (UObject 안전성을 위해 WeakLambda 사용)
            FOnQueryDataComplete Callback;
            Callback.BindWeakLambda(this, [this](UDataAsset* InAsset)
            {
                if (UHktActionDataAsset* ActionAsset = Cast<UHktActionDataAsset>(InAsset))
                {
                    CachedActionAsset = ActionAsset;
                } 
                else 
                {
                    UE_LOG(LogTemp, Error, TEXT("Invalid Action Data Asset: %s"), *InAsset->GetName());
                }
            });
            Provider->QueryDataAssetByTag(FGameplayTag::RequestGameplayTag(FName("ActionTag")), Callback);
        }
    }
}

FGameplayTag UHktCommandContext_BySlot::ResolveEventTag() const
{
    // 데이터가 아직 도착하지 않았다면 nullptr를 반환할 수 있음
    return CachedActionAsset ? CachedActionAsset->EventTag : FGameplayTag();
}

bool UHktCommandContext_BySlot::IsValid() const
{
    // 비동기 로딩이 완료되어 데이터가 유효해지면 true 반환
    return CachedActionAsset != nullptr;
}

bool UHktCommandContext_BySlot::IsRequiredTarget() const
{
    return true;
}

//-----------------------------------------------------------------------------
// Target Context - By Cursor
//-----------------------------------------------------------------------------

void UHktTargetContext_ByClick::Initialize(const FHitResult& InHit)
{
    // 1. 기본 위치 캐싱 (지형 클릭 대비)
    CachedLocation = InHit.Location;
    CachedUnitHandle = FHktUnitHandle(); // Invalid로 초기화

    if (!InHit.bBlockingHit)
    {
        return;
    }
    
    if (UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld()))
    {
        if (const auto& Provider = Service->GetSelectionProvider())
        {
            // 단일 대상 쿼리
            TArray<FHktUnitHandle> HitUnits;
            if (Provider->QuerySelectUnits(InHit, HitUnits) && HitUnits.Num() > 0)
            {
                CachedUnitHandle = HitUnits[0];
                
                // 대상이 유닛이라면, 위치 정보를 유닛의 중심점이나 소켓 위치로 보정할 수 있음
                // 예: CachedLocation = Provider->GetUnitLocation(CachedUnitHandle);
            }
        }
    }
}

FVector UHktTargetContext_ByClick::GetTargetLocation() const
{
    return CachedLocation;
}

FHktUnitHandle UHktTargetContext_ByClick::GetTargetUnit() const
{
    return CachedUnitHandle;
}

bool UHktTargetContext_ByClick::IsValid() const
{
    return true;
}