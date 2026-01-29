// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktVMProcessorComponent.h"
#include "HktRuntimeInterfaces.h"
#include "HktMasterStashComponent.h"
#include "HktVisibleStashComponent.h"
#include "VM/HktVMProcessor.h"

UHktVMProcessorComponent::UHktVMProcessorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UHktVMProcessorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // VMProcessor 인스턴스 생성
    VMProcessor = MakeUnique<FHktVMProcessor>();
}

void UHktVMProcessorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    VMProcessor.Reset();
    bIsInitialized = false;
    MasterStash = nullptr;
    VisibleStash = nullptr;
    
    Super::EndPlay(EndPlayReason);
}

void UHktVMProcessorComponent::InitializeWithMasterStash(UHktMasterStashComponent* InMasterStash)
{
    if (!VMProcessor || !InMasterStash)
    {
        UE_LOG(LogTemp, Warning, TEXT("VMProcessorComponent: Failed to initialize - invalid VMProcessor or MasterStash"));
        return;
    }
    
    MasterStash = InMasterStash;
    VisibleStash = nullptr;
    
    // 컴포넌트가 직접 IStashInterface를 구현하므로 바로 전달
    VMProcessor->Initialize(InMasterStash);
    
    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("VMProcessorComponent: Initialized with MasterStash"));
}

void UHktVMProcessorComponent::InitializeWithVisibleStash(UHktVisibleStashComponent* InVisibleStash)
{
    if (!VMProcessor || !InVisibleStash)
    {
        UE_LOG(LogTemp, Warning, TEXT("VMProcessorComponent: Failed to initialize - invalid VMProcessor or VisibleStash"));
        return;
    }
    
    MasterStash = nullptr;
    VisibleStash = InVisibleStash;
    
    // 컴포넌트가 직접 IStashInterface를 구현하므로 바로 전달
    VMProcessor->Initialize(InVisibleStash);
    
    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("VMProcessorComponent: Initialized with VisibleStash"));
}

void UHktVMProcessorComponent::QueueIntentEvent(const FHktIntentEvent& Event)
{
    if (!VMProcessor || !bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("VMProcessorComponent: Cannot queue event - not initialized"));
        return;
    }
    
    VMProcessor->QueueIntentEvent(Event);
}

void UHktVMProcessorComponent::QueueIntentEvents(const TArray<FHktIntentEvent>& Events)
{
    if (!VMProcessor || !bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("VMProcessorComponent: Cannot queue events - not initialized"));
        return;
    }
    
    for (const FHktIntentEvent& Event : Events)
    {
        VMProcessor->QueueIntentEvent(Event);
    }
}

void UHktVMProcessorComponent::ProcessFrame(int32 CurrentFrame, float DeltaSeconds)
{
    if (!VMProcessor || !bIsInitialized)
    {
        return;
    }
    
    VMProcessor->Tick(CurrentFrame, DeltaSeconds);
}

void UHktVMProcessorComponent::NotifyCollision(FHktEntityId WatchedEntity, FHktEntityId HitEntity)
{
    if (VMProcessor && bIsInitialized)
    {
        VMProcessor->NotifyCollision(WatchedEntity, HitEntity);
    }
}

void UHktVMProcessorComponent::NotifyAnimEnd(FHktEntityId Entity)
{
    if (VMProcessor && bIsInitialized)
    {
        VMProcessor->NotifyAnimEnd(Entity);
    }
}

void UHktVMProcessorComponent::NotifyMoveEnd(FHktEntityId Entity)
{
    if (VMProcessor && bIsInitialized)
    {
        VMProcessor->NotifyMoveEnd(Entity);
    }
}
