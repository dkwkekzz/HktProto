// StateTagComponent.cpp

#include "HktStateTreeComponent.h"
#include "Net/UnrealNetwork.h" // DOREPLIFETIME 포함
#include "DrawDebugHelpers.h"

UHktStateTreeComponent::UHktStateTreeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	// 컴포넌트가 리플리케이트되도록 설정 (필수)
	SetIsReplicatedByDefault(true);
}

void UHktStateTreeComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...
}

void UHktStateTreeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bEnableDebugDrawing)
	{
		const AActor* Owner = GetOwner();
		if (Owner)
		{
			FString TagString = "Tags: ";
			for (const FGameplayTag& Tag : ActiveStateTags)
			{
				TagString += Tag.ToString() + " ";
			}

			FVector Location = Owner->GetActorLocation();
			//DrawDebugString(GetWorld(), FVector(0, 0, 100.f), TagString, Owner, FColor::White, 0.0f);
		}
	}
}

void UHktStateTreeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHktStateTreeComponent, ActiveStateTags);
}

bool UHktStateTreeComponent::HasStateTag(FGameplayTag Tag) const
{
	return ActiveStateTags.HasTag(Tag);
}

// [서버] 태그 추가 RPC 구현
void UHktStateTreeComponent::Server_AddStateTag_Implementation(FGameplayTag TagToAdd)
{
	if (!ActiveStateTags.HasTag(TagToAdd))
	{
		ActiveStateTags.AddTag(TagToAdd);
		
		// 1. [서버] 델리게이트 브로드캐스트
		OnStateTagChanged.Broadcast(TagToAdd, true);
	}
}

// [서버] 태그 제거 RPC 구현
void UHktStateTreeComponent::Server_RemoveStateTag_Implementation(FGameplayTag TagToRemove)
{
	if (ActiveStateTags.HasTag(TagToRemove))
	{
		ActiveStateTags.RemoveTag(TagToRemove);
		
		// 2. [서버] 델리게이트 브로드캐스트
		OnStateTagChanged.Broadcast(TagToRemove, false);
	}
}

// [클라이언트] 태그 복제 시 호출됨
void UHktStateTreeComponent::OnRep_ActiveStateTags(FGameplayTagContainer OldTags)
{
}
