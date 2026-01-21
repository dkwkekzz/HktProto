// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "EnterCharacterFlowDefinition.h"
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
REGISTER_FLOW_DEFINITION(FEnterCharacterFlowDefinition)

// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "FireballFlowDefinition.h"
#include "HktFlowBuilder.h"
#include "HktIntentInterface.h"
#include "HktEntityManager.h"
#include "Flow/FlowDefinitionRegistry.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Event_Skill_Fireball, "Hkt.Event.Skill.Fireball");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Anim_Mage_FireballCast, "Anim.Mage.FireballCast");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Entity_Projectile_Fireball, "Entity.Projectile.Fireball");

bool FFireballFlowDefinition::BuildBytecode(FHktFlowBuilder& Builder, const FHktIntentEvent& Event, FHktEntityManager* EntityManager)
{
	if (!EntityManager)
	{
		return false;
	}

	// 자연어: "시전 -> 파이어볼 생성 -> 전방 이동 -> 충돌 시 폭발 + 범위 데미지"

	constexpr uint8 REG_FIREBALL = 0;
	constexpr uint8 REG_HIT_TARGET = 1;
	constexpr uint8 REG_NEARBY_LIST = 2;
	constexpr uint8 REG_LOOP_TARGET = 3;

	// 1. 시전 애니메이션
	Builder.PlayAnimation(TAG_Anim_Mage_FireballCast);
	Builder.WaitSeconds(1.0f);

	// 2. 파이어볼 생성 (레지스터 0에 저장)
	Builder.SpawnEntity(TAG_Entity_Projectile_Fireball, REG_FIREBALL);

	// 3. 전방 이동 (1500 속도)
	Builder.MoveForward(1500.0f);

	// 4. 충돌 대기 (충돌 상대가 REG_HIT_TARGET에 저장)
	Builder.WaitUntilCollision(REG_HIT_TARGET, 50.0f);

	// 5. 파이어볼 파괴
	Builder.DestroyEntity(REG_FIREBALL);

	// 6. 직격 데미지
	Builder.SetDamage(REG_HIT_TARGET, 100.0f);

	// 7. 폭발 범위 쿼리 (파이어볼 위치 기준)
	Builder.QueryNearby(300.0f, REG_FIREBALL, REG_NEARBY_LIST);

	// 8. ForEach로 범위 내 모든 대상에게 효과 적용
	Builder.ForEachTarget(REG_NEARBY_LIST, REG_LOOP_TARGET);
	{
		// 스플래시 데미지
		Builder.SetDamage(REG_LOOP_TARGET, 50.0f);
		// 화염 DoT
		Builder.ApplyBurning(REG_LOOP_TARGET, 10.0f, 5.0f);
	}
	Builder.EndForEach();

	// 9. 종료
	Builder.End();

	return true;
}

FGameplayTag FFireballFlowDefinition::GetEventTag() const
{
	return TAG_Event_Skill_Fireball;
}

// Auto-register this flow definition
REGISTER_FLOW_DEFINITION(FFireballFlowDefinition)

// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "MoveToLocationFlowDefinition.h"
#include "HktFlowBuilder.h"
#include "HktIntentInterface.h"
#include "HktEntityManager.h"
#include "Flow/FlowDefinitionRegistry.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Event_Action_Move, "Hkt.Event.Action.Move");

bool FMoveToLocationFlowDefinition::BuildBytecode(FHktFlowBuilder& Builder, const FHktIntentEvent& Event, FHktEntityManager* EntityManager)
{
	if (!EntityManager)
	{
		return false;
	}

	// 자연어: "목표 위치로 이동, 도착하면 정지"

	// 1. 목표로 이동 시작
	Builder.MoveTo(Event.Location);

	// 2. 도착까지 대기
	Builder.WaitUntilArrival(10.0f);

	// 3. 정지
	Builder.Stop();

	// 4. 종료
	Builder.End();

	return true;
}

FGameplayTag FMoveToLocationFlowDefinition::GetEventTag() const
{
	return TAG_Event_Action_Move;
}

bool FMoveToLocationFlowDefinition::ValidateEvent(const FHktIntentEvent& Event) const
{
	// Move events should have a valid location
	return Event.Location != FVector::ZeroVector;
}

// Auto-register this flow definition
REGISTER_FLOW_DEFINITION(FMoveToLocationFlowDefinition)
