#include "HktViewHandle.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Actor.h"

// --- Actor Handle ---
void UHktActorViewHandle::Initialize(AActor* InActor)
{
    ManagedActor = InActor;
}

void UHktActorViewHandle::DestroyView()
{
    if (ManagedActor.IsValid())
    {
        ManagedActor->Destroy();
    }
}

// --- Niagara Handle ---
void UHktNiagaraViewHandle::Initialize(UNiagaraComponent* InComponent)
{
    ManagedComponent = InComponent;
}

void UHktNiagaraViewHandle::DestroyView()
{
    if (ManagedComponent.IsValid())
    {
        ManagedComponent->Deactivate();
        ManagedComponent->DestroyComponent();
    }
}

// --- Sound Handle ---
void UHktSoundViewHandle::Initialize(UAudioComponent* InComponent)
{
    ManagedComponent = InComponent;
}

void UHktSoundViewHandle::DestroyView()
{
    if (ManagedComponent.IsValid())
    {
        ManagedComponent->Stop();
        ManagedComponent->DestroyComponent();
    }
}

// --- Widget Handle ---
void UHktWidgetViewHandle::Initialize(UUserWidget* InWidget)
{
    ManagedWidget = InWidget;
}

void UHktWidgetViewHandle::DestroyView()
{
    if (ManagedWidget.IsValid())
    {
        ManagedWidget->RemoveFromParent();
    }
}
