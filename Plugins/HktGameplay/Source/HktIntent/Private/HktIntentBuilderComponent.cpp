// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktIntentBuilderComponent.h"
#include "HktServiceSubsystem.h"
#include "IHktSelectionProvider.h"
#include "HktActionDataAsset.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

//=============================================================================
// UHktIntentBuilderComponent
//=============================================================================

UHktIntentBuilderComponent::UHktIntentBuilderComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

//-----------------------------------------------------------------------------
// Action Creation
//-----------------------------------------------------------------------------

void UHktIntentBuilderComponent::CreateSubjectAction()
{
    FHitResult Hit;
    if (GetHitUnderCursor(Hit))
    {
        CacheSubjectFromHit(Hit);
        
        // 새 Subject 선택 시 Command/Target 초기화
        EventTag = FGameplayTag();
        TargetHandle = FHktUnitHandle();
        TargetLocation = FVector::ZeroVector;
        bTargetRequired = true;
    }
    else
    {
        Reset();
    }
}

void UHktIntentBuilderComponent::CreateCommandAction(int32 SlotIndex)
{
    if (!SubjectHandle.IsValid())
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Command 데이터 조회 (Codex Provider 사용)
    if (UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(World))
    {
        if (const auto& Provider = Service->GetCodexProvider())
        {
            TWeakObjectPtr<UHktIntentBuilderComponent> WeakThis(this);
            
            FOnQueryDataComplete Callback;
            Callback.BindLambda([WeakThis](UDataAsset* InAsset)
            {
                if (UHktIntentBuilderComponent* Comp = WeakThis.Get())
                {
                    if (UHktActionDataAsset* ActionAsset = Cast<UHktActionDataAsset>(InAsset))
                    {
                        Comp->EventTag = ActionAsset->EventTag;
                        Comp->bTargetRequired = true; // ActionAsset에서 가져오도록 확장 가능
                    }
                }
            });
            Provider->QueryActionData(SubjectHandle, Callback);
        }
    }

    // Target 초기화
    TargetHandle = FHktUnitHandle();
    TargetLocation = FVector::ZeroVector;
}

void UHktIntentBuilderComponent::CreateTargetAction()
{
    FHitResult Hit;
    if (GetHitUnderCursor(Hit))
    {
        CacheTargetFromHit(Hit);
    }
}

//-----------------------------------------------------------------------------
// Validation
//-----------------------------------------------------------------------------

bool UHktIntentBuilderComponent::IsValid() const
{
    return SubjectHandle.IsValid() && EventTag.IsValid();
}

bool UHktIntentBuilderComponent::IsReadyToSubmit() const
{
    if (!IsValid())
    {
        return false;
    }

    // Target이 필요한 경우 위치가 설정되어 있어야 함
    if (bTargetRequired)
    {
        return !TargetLocation.IsZero();
    }

    return true;
}

//-----------------------------------------------------------------------------
// Lifecycle
//-----------------------------------------------------------------------------

void UHktIntentBuilderComponent::Reset()
{
    SubjectHandle = FHktUnitHandle();
    EventTag = FGameplayTag();
    TargetHandle = FHktUnitHandle();
    TargetLocation = FVector::ZeroVector;
    bTargetRequired = true;
}

void UHktIntentBuilderComponent::ResetCommand()
{
    EventTag = FGameplayTag();
    TargetHandle = FHktUnitHandle();
    TargetLocation = FVector::ZeroVector;
    bTargetRequired = true;
}

//-----------------------------------------------------------------------------
// Internal Helpers
//-----------------------------------------------------------------------------

bool UHktIntentBuilderComponent::GetHitUnderCursor(FHitResult& OutHit) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    // Owner가 PlayerController인 경우 직접 사용
    if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
    {
        return PC->GetHitResultUnderCursor(ECC_Visibility, false, OutHit);
    }

    // 그 외의 경우 첫 번째 로컬 플레이어 컨트롤러 사용
    if (APlayerController* PC = World->GetFirstPlayerController())
    {
        return PC->GetHitResultUnderCursor(ECC_Visibility, false, OutHit);
    }

    return false;
}

void UHktIntentBuilderComponent::CacheSubjectFromHit(const FHitResult& Hit)
{
    SubjectHandle = FHktUnitHandle();

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    if (UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(World))
    {
        if (const auto& Provider = Service->GetSelectionProvider())
        {
            Provider->QuerySelectUnit(Hit, SubjectHandle);
        }
    }
}

void UHktIntentBuilderComponent::CacheTargetFromHit(const FHitResult& Hit)
{
    TargetLocation = Hit.Location;
    TargetHandle = FHktUnitHandle();

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    if (UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(World))
    {
        if (const auto& Provider = Service->GetSelectionProvider())
        {
            Provider->QuerySelectUnit(Hit, TargetHandle);
        }
    }
}
