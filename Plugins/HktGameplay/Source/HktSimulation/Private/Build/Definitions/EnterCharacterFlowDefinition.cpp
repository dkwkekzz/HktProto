// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "Build/Definitions/EnterCharacterFlowDefinition.h"
#include "Build/FlowDefinitionRegistry.h"
#include "Core/HktVMBuilder.h"
#include "Core/HktAttributeStore.h"
#include "HktIntentInterface.h"
#include "NativeGameplayTags.h"

// 태그 정의
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Event_Entity_EnterCharacter, "Hkt.Event.Entity.EnterCharacter");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Entity_Character, "Entity.Character");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Entity_Character_Equipment, "Entity.Character.Equipment");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Anim_Character_Spawn, "Anim.Character.Spawn");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Anim_Character_Spawn_Intro, "Anim.Character.Spawn_Intro");

// 자동 등록
REGISTER_FLOW_DEFINITION(FEnterCharacterFlowDefinition)

FGameplayTag FEnterCharacterFlowDefinition::GetEventTag() const
{
    return TAG_Event_Entity_EnterCharacter;
}

bool FEnterCharacterFlowDefinition::BuildBytecode(
    FHktVMBuilder& Builder, 
    const FHktIntentEvent& Event, 
    FHktAttributeStore* Attributes)
{
    // ========================================================================
    // 캐릭터 입장 Flow
    // 
    // 자연어로 읽으면:
    // "캐릭터를 생성하고 스폰 애니메이션을 재생한다.
    //  0.5초 후 장비를 생성하고 인트로 애니메이션을 재생한다."
    // ========================================================================
    
    // 레지스터 할당
    constexpr uint8 REG_CHARACTER = 0;    // "캐릭터"
    constexpr uint8 REG_EQUIPMENT = 1;    // "장비"
    
    // "캐릭터를 생성하고"
    Builder.SpawnEntity(TAG_Entity_Character, REG_CHARACTER);
    
    // "스폰 애니메이션을 재생한다"
    Builder.PlayAnimation(TAG_Anim_Character_Spawn);
    
    // "0.5초 후"
    Builder.WaitSeconds(0.5f);
    
    // "장비를 생성하고"
    Builder.SpawnEntity(TAG_Entity_Character_Equipment, REG_EQUIPMENT);
    
    // "인트로 애니메이션을 재생한다"
    Builder.PlayAnimation(TAG_Anim_Character_Spawn_Intro);
    
    // "끝"
    Builder.End();
    
    return true;
}
