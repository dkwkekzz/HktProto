#include "HktDataAsset.h"
#include "HktViewHandle.h"
#include "Engine/World.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"

UHktViewHandle* UHktActorDataAsset::CreateViewHandle(UObject* WorldContextObject, const FTransform& SpawnTransform) const
{
    UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
    if (!World || !ActorClass)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass, SpawnTransform, SpawnParams);

    if (SpawnedActor)
    {
        // 핸들은 일시적인 객체이므로 GetTransientPackage()를 오너로 지정하여 가비지 컬렉션을 방지합니다.
        UHktActorViewHandle* ActorHandle = NewObject<UHktActorViewHandle>(GetTransientPackage());
        ActorHandle->Initialize(SpawnedActor);
        return ActorHandle;
    }
    return nullptr;
}

UHktViewHandle* UHktNiagaraDataAsset::CreateViewHandle(UObject* WorldContextObject, const FTransform& SpawnTransform) const
{
    UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
    if (!World || !NiagaraSystem)
    {
        return nullptr;
    }

    UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, NiagaraSystem, SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator(), SpawnTransform.GetScale3D());

    if (NiagaraComponent)
    {
        UHktNiagaraViewHandle* NiagaraHandle = NewObject<UHktNiagaraViewHandle>(GetTransientPackage());
        NiagaraHandle->Initialize(NiagaraComponent);
        return NiagaraHandle;
    }
    return nullptr;
}

UHktViewHandle* UHktSoundDataAsset::CreateViewHandle(UObject* WorldContextObject, const FTransform& SpawnTransform) const
{
    UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
    if (!World || !SoundAsset)
    {
        return nullptr;
    }

    UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAtLocation(World, SoundAsset, SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator());

    if (AudioComponent)
    {
        UHktSoundViewHandle* SoundHandle = NewObject<UHktSoundViewHandle>(GetTransientPackage());
        SoundHandle->Initialize(AudioComponent);
        return SoundHandle;
    }
    return nullptr;
}

UHktViewHandle* UHktWidgetDataAsset::CreateViewHandle(UObject* WorldContextObject, const FTransform& SpawnTransform) const
{
    UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
    if (!World || !WidgetClass)
    {
        return nullptr;
    }

    UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(World, WidgetClass);

    if (CreatedWidget)
    {
        CreatedWidget->AddToViewport();
        UHktWidgetViewHandle* WidgetHandle = NewObject<UHktWidgetViewHandle>(GetTransientPackage());
        WidgetHandle->Initialize(CreatedWidget);
        return WidgetHandle;
    }
    return nullptr;
}

