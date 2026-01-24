// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "Build/Definitions/FireballFlowDefinition.h"
#include "Build/FlowDefinitionRegistry.h"
#include "Core/HktVMBuilder.h"
#include "Core/HktAttributeStore.h"
#include "HktIntentInterface.h"
#include "NativeGameplayTags.h"

// 태그 정의
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Event_Skill_Fireball, "Hkt.Event.Skill.Fireball");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Anim_Mage_FireballCast, "Anim.Mage.FireballCast");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Entity_Projectile_Fireball, "Entity.Projectile.Fireball");

// 자동 등록
REGISTER_FLOW_DEFINITION(FFireballFlowDefinition)

FGameplayTag FFireballFlowDefinition::GetEventTag() const
{
    return TAG_Event_Skill_Fireball;
}

bool FFireballFlowDefinition::BuildBytecode(
    FHktVMBuilder& Builder, 
    const FHktIntentEvent& Event, 
    FHktAttributeStore* Attributes)
{
    // ========================================================================
    // 파이어볼 스킬 Flow
    // 
    // 자연어로 읽으면:
    // "시전 애니메이션을 재생하고 1초 기다린다.
    //  파이어볼을 생성하여 앞으로 날린다.
    //  충돌하면 파이어볼을 제거하고 직격 대상에게 100 피해를 준다.
    //  주변 300 범위 내 대상들에게 각각 50 피해와 화상을 입힌다."
    // ========================================================================
    
    // 레지스터 할당 (자연어의 "~을/를"에 대응)
    constexpr uint8 REG_FIREBALL = 0;      // "파이어볼"
    constexpr uint8 REG_HIT_TARGET = 1;    // "맞은 대상"
    constexpr uint8 REG_NEARBY_LIST = 2;   // "주변 대상들"
    constexpr uint8 REG_LOOP_TARGET = 3;   // "각 대상"
    
    // === 시전 단계 ===
    // "시전 애니메이션을 재생하고"
    Builder.PlayAnimation(TAG_Anim_Mage_FireballCast);
    
    // "1초 기다린다"
    Builder.WaitSeconds(1.0f);
    
    // === 투사체 단계 ===
    // "파이어볼을 생성하여"
    Builder.SpawnEntity(TAG_Entity_Projectile_Fireball, REG_FIREBALL);
    
    // "앞으로 날린다"
    Builder.MoveForward(1500.0f);
    
    // "충돌할 때까지 기다린다"
    Builder.WaitUntilCollision(REG_HIT_TARGET, 10.0f);
    
    // === 폭발 단계 ===
    // "파이어볼을 제거하고"
    Builder.DestroyEntity(REG_FIREBALL);
    
    // "직격 대상에게 100 피해를 준다"
    Builder.SetDamage(REG_HIT_TARGET, 100.0f);
    
    // === 범위 효과 단계 ===
    // "주변 300 범위 내 대상들을 찾는다"
    Builder.QueryNearby(300.0f, REG_HIT_TARGET, REG_NEARBY_LIST);
    
    // "각 대상에 대해..."
    Builder.ForEachTarget(REG_NEARBY_LIST, REG_LOOP_TARGET);
    {
        // "50 피해를 입힌다"
        Builder.SetDamage(REG_LOOP_TARGET, 50.0f);
        
        // "화상을 입힌다"
        Builder.ApplyBurning(REG_LOOP_TARGET, 10.0f, 5.0f);
    }
    Builder.EndForEach();
    
    // "끝"
    Builder.End();
    
    return true;
}
