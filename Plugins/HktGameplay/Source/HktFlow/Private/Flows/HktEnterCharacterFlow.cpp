// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "Flows/HktEnterCharacterBehavior.h"
#include "HktFlowBuilder.h"
#include "GameplayTagsManager.h"

FGameplayTag UHktEnterCharacterBehavior::GetStaticEventTag()
{
	return FGameplayTag::RequestGameplayTag(FName("Event.Character.Enter"));
}

FGameplayTag UHktEnterCharacterBehavior::GetEventTag() const
{
	return GetStaticEventTag();
}

void UHktEnterCharacterBehavior::DefineFlow(FHktFlowBuilder& Flow, const void* EventData)
{
	const FHktEnterCharacterEventData* Data = static_cast<const FHktEnterCharacterEventData*>(EventData);
	if (!Data)
	{
		return;
	}

	/*
	 * 캐릭터 진입 흐름:
	 * 
	 * 1. 캐릭터 엔티티 생성
	 * 2. 생성되면 → 장착 아이템들 부착
	 * 3. 등장 애니메이션 재생
	 */

	Flow.SpawnEntity(Data->CharacterType, Data->SpawnLocation)
		.OnSpawned([&, Data](int32 CharacterHandle)
		{
			// 장착 아이템들을 캐릭터에 부착
			for (const FName& ItemType : Data->EquippedItems)
			{
				Flow.SpawnEntity(ItemType, CharacterHandle);
			}

			// 등장 연출 애니메이션
			Flow.PlayAnimation(CharacterHandle, FName("Spawn_Intro"));
		});
}

