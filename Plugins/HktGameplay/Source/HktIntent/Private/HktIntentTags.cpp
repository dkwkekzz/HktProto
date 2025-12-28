// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktIntentTags.h"

namespace HktIntentTags
{
	// Root
	UE_DEFINE_GAMEPLAY_TAG(Hkt, "Hkt");

	//==========================================================================
	// Event Tags - Subject에 부여되는 이벤트
	//==========================================================================

	UE_DEFINE_GAMEPLAY_TAG(Event, "Hkt.Event");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character, "Hkt.Event.Character");
	UE_DEFINE_GAMEPLAY_TAG(Event_MoveToLocation, "Hkt.Event.MoveToLocation");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Animation, "Hkt.Event.Animation");
	UE_DEFINE_GAMEPLAY_TAG(Event_Animation_Move, "Hkt.Event.Animation.Move");

	// State: 지속 시간이 있는 상태 이벤트
	UE_DEFINE_GAMEPLAY_TAG(Event_State, "Hkt.Event.State");
	UE_DEFINE_GAMEPLAY_TAG(Event_State_Idle, "Hkt.Event.State.Idle");
	UE_DEFINE_GAMEPLAY_TAG(Event_State_Moving, "Hkt.Event.State.Moving");
	UE_DEFINE_GAMEPLAY_TAG(Event_State_Casting, "Hkt.Event.State.Casting");
	UE_DEFINE_GAMEPLAY_TAG(Event_State_Attacking, "Hkt.Event.State.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(Event_State_Stunned, "Hkt.Event.State.Stunned");
	UE_DEFINE_GAMEPLAY_TAG(Event_State_Burning, "Hkt.Event.State.Burning");
	UE_DEFINE_GAMEPLAY_TAG(Event_State_Frozen, "Hkt.Event.State.Frozen");
	UE_DEFINE_GAMEPLAY_TAG(Event_State_Dead, "Hkt.Event.State.Dead");

	// Action: 즉시 처리되는 사건 이벤트
	UE_DEFINE_GAMEPLAY_TAG(Event_Action, "Hkt.Event.Action");
	UE_DEFINE_GAMEPLAY_TAG(Event_Action_Spawn, "Hkt.Event.Action.Spawn");
	UE_DEFINE_GAMEPLAY_TAG(Event_Action_Destroy, "Hkt.Event.Action.Destroy");
	UE_DEFINE_GAMEPLAY_TAG(Event_Action_Hit, "Hkt.Event.Action.Hit");
	UE_DEFINE_GAMEPLAY_TAG(Event_Action_Heal, "Hkt.Event.Action.Heal");
	UE_DEFINE_GAMEPLAY_TAG(Event_Action_Explode, "Hkt.Event.Action.Explode");

	// Ability: 능력 관련 이벤트
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability, "Hkt.Event.Ability");
	// Fireball
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_Fireball, "Hkt.Event.Ability.Fireball");
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_Fireball_Cast, "Hkt.Event.Ability.Fireball.Cast");
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_Fireball_Fire, "Hkt.Event.Ability.Fireball.Fire");
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_Fireball_Explode, "Hkt.Event.Ability.Fireball.Explode");
	// Melee
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_Melee, "Hkt.Event.Ability.Melee");
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_Melee_Attack, "Hkt.Event.Ability.Melee.Attack");
	// Summon
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_Summon, "Hkt.Event.Ability.Summon");
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_Summon_Cast, "Hkt.Event.Ability.Summon.Cast");
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_Summon_Spawn, "Hkt.Event.Ability.Summon.Spawn");

	// Movement: 이동 관련 이벤트
	UE_DEFINE_GAMEPLAY_TAG(Event_Movement, "Hkt.Event.Movement");
	UE_DEFINE_GAMEPLAY_TAG(Event_Movement_Forward, "Hkt.Event.Movement.Forward");
	UE_DEFINE_GAMEPLAY_TAG(Event_Movement_Chase, "Hkt.Event.Movement.Chase");
	UE_DEFINE_GAMEPLAY_TAG(Event_Movement_Flee, "Hkt.Event.Movement.Flee");
	UE_DEFINE_GAMEPLAY_TAG(Event_Movement_Patrol, "Hkt.Event.Movement.Patrol");

	//==========================================================================
	// Subject Tags - Subject 타입 태그
	//==========================================================================

	UE_DEFINE_GAMEPLAY_TAG(Subject, "Hkt.Subject");
	// Unit types
	UE_DEFINE_GAMEPLAY_TAG(Subject_Unit, "Hkt.Subject.Unit");
	UE_DEFINE_GAMEPLAY_TAG(Subject_Unit_Player, "Hkt.Subject.Unit.Player");
	UE_DEFINE_GAMEPLAY_TAG(Subject_Unit_Enemy, "Hkt.Subject.Unit.Enemy");
	UE_DEFINE_GAMEPLAY_TAG(Subject_Unit_NPC, "Hkt.Subject.Unit.NPC");
	// Projectile types
	UE_DEFINE_GAMEPLAY_TAG(Subject_Projectile, "Hkt.Subject.Projectile");
	UE_DEFINE_GAMEPLAY_TAG(Subject_Projectile_Fireball, "Hkt.Subject.Projectile.Fireball");
	UE_DEFINE_GAMEPLAY_TAG(Subject_Projectile_Arrow, "Hkt.Subject.Projectile.Arrow");
	// Effect types
	UE_DEFINE_GAMEPLAY_TAG(Subject_Effect, "Hkt.Subject.Effect");
	UE_DEFINE_GAMEPLAY_TAG(Subject_Effect_Explosion, "Hkt.Subject.Effect.Explosion");
	UE_DEFINE_GAMEPLAY_TAG(Subject_Effect_AOE, "Hkt.Subject.Effect.AOE");
	// Summon types
	UE_DEFINE_GAMEPLAY_TAG(Subject_Summon, "Hkt.Subject.Summon");

	//==========================================================================
	// Damage Type Tags
	//==========================================================================

	UE_DEFINE_GAMEPLAY_TAG(DamageType, "Hkt.DamageType");
	UE_DEFINE_GAMEPLAY_TAG(DamageType_Physical, "Hkt.DamageType.Physical");
	UE_DEFINE_GAMEPLAY_TAG(DamageType_Fire, "Hkt.DamageType.Fire");
	UE_DEFINE_GAMEPLAY_TAG(DamageType_Ice, "Hkt.DamageType.Ice");
	UE_DEFINE_GAMEPLAY_TAG(DamageType_Lightning, "Hkt.DamageType.Lightning");
	UE_DEFINE_GAMEPLAY_TAG(DamageType_Holy, "Hkt.DamageType.Holy");
	UE_DEFINE_GAMEPLAY_TAG(DamageType_Dark, "Hkt.DamageType.Dark");

	//==========================================================================
	// Input Tags - 플레이어 입력 이벤트 (기존 호환)
	//==========================================================================

	// Input Root
	UE_DEFINE_GAMEPLAY_TAG(Input, "Hkt.Input");

	// Movement Events
	UE_DEFINE_GAMEPLAY_TAG(Input_Movement, "Hkt.Input.Movement");
	UE_DEFINE_GAMEPLAY_TAG(Input_Movement_Move, "Hkt.Input.Movement.Move");
	UE_DEFINE_GAMEPLAY_TAG(Input_Movement_Stop, "Hkt.Input.Movement.Stop");
	UE_DEFINE_GAMEPLAY_TAG(Input_Movement_Patrol, "Hkt.Input.Movement.Patrol");

	// Combat Events
	UE_DEFINE_GAMEPLAY_TAG(Input_Combat, "Hkt.Input.Combat");
	UE_DEFINE_GAMEPLAY_TAG(Input_Combat_Attack, "Hkt.Input.Combat.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Input_Combat_AttackMove, "Hkt.Input.Combat.AttackMove");
	UE_DEFINE_GAMEPLAY_TAG(Input_Combat_Skill, "Hkt.Input.Combat.Skill");
	UE_DEFINE_GAMEPLAY_TAG(Input_Combat_Hold, "Hkt.Input.Combat.Hold");

	// Selection Events
	UE_DEFINE_GAMEPLAY_TAG(Input_Selection, "Hkt.Input.Selection");
	UE_DEFINE_GAMEPLAY_TAG(Input_Selection_Select, "Hkt.Input.Selection.Select");
	UE_DEFINE_GAMEPLAY_TAG(Input_Selection_Deselect, "Hkt.Input.Selection.Deselect");
	UE_DEFINE_GAMEPLAY_TAG(Input_Selection_BoxSelect, "Hkt.Input.Selection.BoxSelect");
	UE_DEFINE_GAMEPLAY_TAG(Input_Selection_SelectAll, "Hkt.Input.Selection.SelectAll");

	// Command Events
	UE_DEFINE_GAMEPLAY_TAG(Input_Command, "Hkt.Input.Command");
	UE_DEFINE_GAMEPLAY_TAG(Input_Command_Custom, "Hkt.Input.Command.Custom");
	UE_DEFINE_GAMEPLAY_TAG(Input_Command_Group, "Hkt.Input.Command.Group");

	// Camera Events
	UE_DEFINE_GAMEPLAY_TAG(Input_Camera, "Hkt.Input.Camera");
	UE_DEFINE_GAMEPLAY_TAG(Input_Camera_Zoom, "Hkt.Input.Camera.Zoom");
	UE_DEFINE_GAMEPLAY_TAG(Input_Camera_Pan, "Hkt.Input.Camera.Pan");
	UE_DEFINE_GAMEPLAY_TAG(Input_Camera_Rotate, "Hkt.Input.Camera.Rotate");

	// UI Events
	UE_DEFINE_GAMEPLAY_TAG(Input_UI, "Hkt.Input.UI");
	UE_DEFINE_GAMEPLAY_TAG(Input_UI_Confirm, "Hkt.Input.UI.Confirm");
	UE_DEFINE_GAMEPLAY_TAG(Input_UI_Cancel, "Hkt.Input.UI.Cancel");
	UE_DEFINE_GAMEPLAY_TAG(Input_UI_Menu, "Hkt.Input.UI.Menu");

	// Spawn Events
	UE_DEFINE_GAMEPLAY_TAG(Input_Spawn, "Hkt.Input.Spawn");
	UE_DEFINE_GAMEPLAY_TAG(Input_Spawn_Unit, "Hkt.Input.Spawn.Unit");
	UE_DEFINE_GAMEPLAY_TAG(Input_Spawn_Building, "Hkt.Input.Spawn.Building");

	//==========================================================================
	// Animation Tags
	//==========================================================================

	UE_DEFINE_GAMEPLAY_TAG(Anim, "Hkt.Anim");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Idle, "Hkt.Anim.Idle");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Moving, "Hkt.Anim.Moving");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Attack, "Hkt.Anim.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Cast, "Hkt.Anim.Cast");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Death, "Hkt.Anim.Death");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Hit, "Hkt.Anim.Hit");

	//==========================================================================
	// VFX Tags
	//==========================================================================

	UE_DEFINE_GAMEPLAY_TAG(VFX, "Hkt.VFX");
	UE_DEFINE_GAMEPLAY_TAG(VFX_Hit, "Hkt.VFX.Hit");
	UE_DEFINE_GAMEPLAY_TAG(VFX_Hit_Fire, "Hkt.VFX.Hit.Fire");
	UE_DEFINE_GAMEPLAY_TAG(VFX_MeleeHit, "Hkt.VFX.MeleeHit");
	UE_DEFINE_GAMEPLAY_TAG(VFX_Explosion, "Hkt.VFX.Explosion");
	UE_DEFINE_GAMEPLAY_TAG(VFX_Summon, "Hkt.VFX.Summon");
	UE_DEFINE_GAMEPLAY_TAG(VFX_Projectile, "Hkt.VFX.Projectile");
	UE_DEFINE_GAMEPLAY_TAG(VFX_Projectile_Fireball, "Hkt.VFX.Projectile.Fireball");

	//==========================================================================
	// Legacy Ability Tags (deprecated)
	//==========================================================================

	UE_DEFINE_GAMEPLAY_TAG(Ability, "Hkt.Ability");
	UE_DEFINE_GAMEPLAY_TAG(Ability_MeleeAttack, "Hkt.Ability.MeleeAttack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Fireball, "Hkt.Ability.Fireball");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Summon, "Hkt.Ability.Summon");
}

