// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "Flow/Definitions/EnterCharacterFlowDefinition.h"
#include "HktFlowBuilder.h"
#include "HktIntentInterface.h"
#include "HktEntityManager.h"
#include "Flow/FlowDefinitionRegistry.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Event_Entity_EnterCharacter, "Hkt.Event.Entity.EnterCharacter");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Entity_Character, "Entity.Character");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Entity_Character_Equipment, "Entity.Character.Equipment");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Anim_Character_Spawn, "Anim.Character.Spawn");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Anim_Character_Spawn_Intro, "Anim.Character.Spawn_Intro");

bool FEnterCharacterFlowDefinition::BuildBytecode(FHktFlowBuilder& Builder, const FHktIntentEvent& Event, FHktEntityManager* EntityManager)
{
	if (!EntityManager)
	{
		return false;
	}

	// 자연어: "캐릭터 생성 -> 스폰 애니메이션 -> 장비 생성 -> 인트로 애니메이션"

	constexpr uint8 REG_CHARACTER = 0;
	constexpr uint8 REG_EQUIPMENT = 1;

	// 1. 캐릭터 엔티티 생성
	Builder.SpawnEntity(TAG_Entity_Character, REG_CHARACTER);

	// 2. 스폰 애니메이션
	Builder.PlayAnimation(TAG_Anim_Character_Spawn);
	Builder.WaitSeconds(0.5f);

	// 3. 장비 엔티티 생성
	Builder.SpawnEntity(TAG_Entity_Character_Equipment, REG_EQUIPMENT);

	// 4. 인트로 애니메이션
	Builder.PlayAnimation(TAG_Anim_Character_Spawn_Intro);

	// 5. 종료
	Builder.End();

	return true;
}

FGameplayTag FEnterCharacterFlowDefinition::GetEventTag() const
{
	return TAG_Event_Entity_EnterCharacter;
}

// Auto-register this flow definition
// REGISTER_FLOW_DEFINITION(FEnterCharacterFlowDefinition)
