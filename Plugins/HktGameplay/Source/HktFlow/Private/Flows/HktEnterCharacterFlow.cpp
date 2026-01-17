// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "Flows/HktEnterCharacterBehavior.h"
#include "HktJobBuilder.h"

void UHktEnterCharacterBehavior::DefineFlow(FHktJobBuilder& Builder, const FHktIntentEvent& Event)
{
	/*
	 * 캐릭터 진입 흐름:
	 * 
	 * 1. 캐릭터 엔티티 생성
	 * 2. 생성되면 → 장착 아이템들 부착
	 * 3. 등장 애니메이션 재생
	 */

	Builder.SpawnEntity(Event.Target.Value)
		.Then(Builder.PlayAnimation(Event.Subject.Value, FGameplayTag::RequestGameplayTag(FName("Anim.Character.Spawn"))))
		.Then(Builder.SpawnEntity(FGameplayTag::RequestGameplayTag(FName("Entity.Character.Equipment"))))
		.Then(Builder.PlayAnimation(Event.Subject.Value, FGameplayTag::RequestGameplayTag(FName("Anim.Character.Spawn_Intro"))));
}

