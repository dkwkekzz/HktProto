#include "HktIntentPlayerState.h"
#include "HktIntentTags.h"

AHktIntentPlayerState::AHktIntentPlayerState()
{
}

void AHktIntentPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

TArray<FHktUnitHandle> AHktIntentPlayerState::ResolveSubjects() const
{
	return TArray<FHktUnitHandle>({ PlayerHandle });
}

FHktUnitHandle AHktIntentPlayerState::ResolvePrimarySubject() const
{
	return PlayerHandle;
}

bool AHktIntentPlayerState::IsValid() const
{
	return PlayerHandle.IsValid();
}

FGameplayTag AHktIntentPlayerState::ResolveEventTag() const
{
	// 플레이어가 캐릭터를 스폰하기 위한 이벤트 태그 반환
	// 서버로부터 받을 정보(소환할 캐릭터 어셋 정보)는 PlayerInitialEventTag에 저장됨
	return PlayerInitialEventTag;
}