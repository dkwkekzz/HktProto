// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

/**
 * Native GameplayTags for the Intent module.
 * 
 * 태그 체계:
 * - Hkt.Event: Subject에 부여되는 이벤트 (State/Action/Ability)
 * - Hkt.Subject: Subject 타입 태그
 * - Hkt.Input: 플레이어 입력 이벤트
 * - Hkt.Anim: 애니메이션 상태
 * - Hkt.VFX: 시각 효과
 * - Hkt.DamageType: 데미지 타입
 */
namespace HktIntentTags
{
	// Root
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Hkt);

	//==========================================================================
	// Event Tags - Subject에 부여되는 이벤트
	//==========================================================================

	// Event Root
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event);

	//--- Character Events ---
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Character);

	//--- Specific Movement Action ---
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_MoveToLocation);

	//--- Animation Events (Event driven) ---
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Animation);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Animation_Move);

	//--- State: 지속 시간이 있는 상태 이벤트 ---
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_State);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_State_Idle);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_State_Moving);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_State_Casting);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_State_Attacking);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_State_Stunned);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_State_Burning);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_State_Frozen);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_State_Dead);

	//--- Action: 즉시 처리되는 사건 이벤트 ---
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Action);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Action_Spawn);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Action_Destroy);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Action_Hit);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Action_Heal);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Action_Explode);

	//--- Ability: 능력 관련 이벤트 ---
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability);
	// Fireball
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_Fireball);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_Fireball_Cast);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_Fireball_Fire);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_Fireball_Explode);
	// Melee
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_Melee);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_Melee_Attack);
	// Summon
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_Summon);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_Summon_Cast);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Ability_Summon_Spawn);

	//--- Movement: 이동 관련 이벤트 ---
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Movement);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Movement_Forward);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Movement_Chase);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Movement_Flee);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Movement_Patrol);

	//==========================================================================
	// Subject Tags - Subject 타입 태그
	//==========================================================================

	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Subject);
	// Unit types
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Subject_Unit);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Subject_Unit_Player);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Subject_Unit_Enemy);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Subject_Unit_NPC);
	// Projectile types
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Subject_Projectile);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Subject_Projectile_Fireball);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Subject_Projectile_Arrow);
	// Effect types
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Subject_Effect);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Subject_Effect_Explosion);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Subject_Effect_AOE);
	// Summon types
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Subject_Summon);

	//==========================================================================
	// Damage Type Tags
	//==========================================================================

	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(DamageType);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(DamageType_Physical);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(DamageType_Fire);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(DamageType_Ice);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(DamageType_Lightning);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(DamageType_Holy);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(DamageType_Dark);

	//==========================================================================
	// Input Tags - 플레이어 입력 이벤트 (기존 호환)
	//==========================================================================

	// Input Root
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input);

	// Movement Events
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Movement);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Movement_Move);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Movement_Stop);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Movement_Patrol);

	// Combat Events
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Combat);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Combat_Attack);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Combat_AttackMove);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Combat_Skill);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Combat_Hold);

	// Selection Events
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Selection);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Selection_Select);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Selection_Deselect);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Selection_BoxSelect);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Selection_SelectAll);

	// Command Events (numbered hotkeys)
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Command);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Command_Custom);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Command_Group);

	// Camera Events
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Camera);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Camera_Zoom);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Camera_Pan);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Camera_Rotate);

	// UI Events
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_UI);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_UI_Confirm);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_UI_Cancel);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_UI_Menu);

	// Spawn Events
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Spawn);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Spawn_Unit);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Spawn_Building);

	//==========================================================================
	// Animation Tags
	//==========================================================================

	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Idle);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Moving);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Attack);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Cast);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Death);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Hit);

	//==========================================================================
	// VFX Tags
	//==========================================================================

	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(VFX);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(VFX_Hit);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(VFX_Hit_Fire);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(VFX_MeleeHit);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(VFX_Explosion);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(VFX_Summon);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(VFX_Projectile);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(VFX_Projectile_Fireball);

	//==========================================================================
	// Legacy Ability Tags (deprecated, use Event_Ability instead)
	//==========================================================================

	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_MeleeAttack);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Fireball);
	HKTINTENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Summon);
}

