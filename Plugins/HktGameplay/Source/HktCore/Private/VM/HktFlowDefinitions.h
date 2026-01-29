#pragma once

#include "HktVMProgram.h"
#include "HktVMStore.h"

/**
 * Flow 정의 예제
 * 
 * FlowBuilder API를 사용해 자연어처럼 읽히는 스킬/행동 로직을 정의합니다.
 * 콜백이나 상태 머신 없이 위에서 아래로 선형으로 읽힙니다.
 */
namespace FlowDefinitions
{
    /**
     * ================================================================
     * 파이어볼 스킬 Flow
     * 
     * 자연어로 읽으면:
     * "시전 애니메이션을 재생하고 1초 기다린다.
     *  파이어볼을 생성하여 앞으로 날린다.
     *  충돌하면 파이어볼을 제거하고 직격 대상에게 100 피해를 준다.
     *  주변 300 범위 내 대상들에게 각각 50 피해와 화상을 입힌다."
     * ================================================================
     */
    inline void RegisterFireball()
    {
        using namespace Reg;
        
        Flow("Ability.Skill.Fireball")
            // === 시전 시작 ===
            .Log("Fireball: 시전 시작")
            .PlayAnim(Self, "CastFireball")
            .WaitSeconds(1.0f)                          // 1초 대기
            
            // === 파이어볼 생성 및 발사 ===
            .Log("Fireball: 투사체 생성")
            .SpawnEntity("/Game/Projectiles/BP_Fireball")  // Spawned = 새 파이어볼
            
            // 파이어볼 위치를 시전자 위치로 설정
            .GetPosition(R0, Self)                      // R0,R1,R2 = 시전자 위치
            .SetPosition(Spawned, R0)                   // 파이어볼 위치 = 시전자 위치
            
            // 파이어볼을 전방으로 이동 (속도 500 cm/s)
            .MoveForward(Spawned, 500)
            .PlaySound("/Game/Sounds/FireballLaunch")
            
            // === 충돌 대기 ===
            .Log("Fireball: 충돌 대기 중...")
            .WaitCollision(Spawned)                     // 충돌 시 Hit = 충돌 대상
            
            // === 충돌 처리 ===
            .Log("Fireball: 충돌! 폭발 처리")
            
            // 파이어볼 위치 저장 (폭발 위치)
            .GetPosition(R3, Spawned)                   // R3,R4,R5 = 폭발 위치
            
            // 파이어볼 제거
            .DestroyEntity(Spawned)
            
            // 직격 대상에게 100 피해
            .ApplyDamageConst(Hit, 100)
            .PlayVFXAttached(Hit, "/Game/VFX/DirectHit")
            
            // 폭발 이펙트
            .PlayVFX(R3, "/Game/VFX/FireballExplosion")
            .PlaySoundAtLocation(R3, "/Game/Sounds/Explosion")
            
            // === 범위 피해 (반경 300cm) ===
            .Log("Fireball: 범위 피해 적용")
            
            // R3에 저장된 위치를 중심으로 범위 검색을 위해
            // 임시 엔티티로 Spawned 레지스터 활용 (이미 제거됨)
            // 대신 Self 기준으로 검색 (시전자 주변 = 폭발 위치 근처 가정)
            // 실제로는 폭발 위치 기준으로 검색해야 하지만, 
            // 여기서는 Hit 엔티티 기준으로 검색
            
            .ForEachInRadius(Hit, 300)                  // Hit 주변 300cm 내 적들
                .Move(Target, Iter)                     // Target = 현재 순회 대상
                .ApplyDamageConst(Target, 50)           // 50 피해
                .ApplyEffect(Target, "Effect.Burn")     // 화상 적용
            .EndForEach()
            
            .Log("Fireball: 완료")
            .Halt()
            .BuildAndRegister();
    }
    
    /**
     * ================================================================
     * 위치 이동 Flow
     * 
     * 자연어로 읽으면:
     * "목표 위치로 이동하고, 도착하면 정지한다."
     * ================================================================
     */
    inline void RegisterMoveTo()
    {
        using namespace Reg;
        
        Flow("Action.Move.ToLocation")
            .Log("MoveTo: 이동 시작")
            
            // 목표 위치 로드 (IntentEvent에서 설정됨)
            .LoadStore(R0, PropertyId::TargetPosX)
            .LoadStore(R1, PropertyId::TargetPosY)
            .LoadStore(R2, PropertyId::TargetPosZ)
            
            // 이동 애니메이션 시작
            .PlayAnim(Self, "Run")
            
            // 목표 위치로 이동 시작 (속도 300 cm/s)
            .MoveToward(Self, R0, 300)
            
            // 이동 완료 대기
            .WaitMoveEnd(Self)
            
            // 정지
            .StopMovement(Self)
            .PlayAnim(Self, "Idle")
            
            .Log("MoveTo: 도착")
            .Halt()
            .BuildAndRegister();
    }
    
    /**
     * ================================================================
     * 캐릭터 입장 Flow
     * 
     * 자연어로 읽으면:
     * "캐릭터를 생성하고 스폰 애니메이션을 재생한다.
     *  0.5초 후 장비를 생성하고 인트로 애니메이션을 재생한다."
     * ================================================================
     */
    inline void RegisterCharacterSpawn()
    {
        using namespace Reg;
        
        Flow("Event.Character.Spawn")
            .Log("CharacterSpawn: 캐릭터 생성")
            
            // 캐릭터 스폰
            .SpawnEntity("/Game/Characters/BP_PlayerCharacter")
            .Move(Self, Spawned)                        // Self = 새로 생성된 캐릭터
            
            // 스폰 위치 설정 (IntentEvent에서)
            .LoadStore(R0, PropertyId::TargetPosX)
            .LoadStore(R1, PropertyId::TargetPosY)
            .LoadStore(R2, PropertyId::TargetPosZ)
            .SetPosition(Self, R0)
            
            // 스폰 이펙트
            .PlayVFXAttached(Self, "/Game/VFX/SpawnEffect")
            .PlaySound("/Game/Sounds/Spawn")
            
            // 스폰 애니메이션
            .PlayAnim(Self, "Spawn")
            
            // 0.5초 대기
            .WaitSeconds(0.5f)
            
            // === 장비 생성 ===
            .Log("CharacterSpawn: 장비 생성")
            
            // 메인 무기 (슬롯 0)
            .SpawnEquipment(Self, 0, "/Game/Weapons/BP_Sword")
            .PlayVFXAttached(Spawned, "/Game/VFX/EquipGlow")
            
            // 보조 장비 (슬롯 1)
            .SpawnEquipment(Self, 1, "/Game/Equipment/BP_Shield")
            
            // 인트로 애니메이션
            .PlayAnimMontage(Self, "IntroMontage")
            .WaitAnimEnd(Self)
            
            // 준비 완료 - Idle 상태로 전환
            .PlayAnim(Self, "Idle")
            
            .Log("CharacterSpawn: 준비 완료")
            .Halt()
            .BuildAndRegister();
    }
    
    /**
     * ================================================================
     * 추가 예제: 기본 공격 Flow
     * 
     * 자연어로 읽으면:
     * "공격 애니메이션을 재생하고, 애니메이션이 끝나면
     *  대상에게 공격력만큼 피해를 준다."
     * ================================================================
     */
    inline void RegisterBasicAttack()
    {
        using namespace Reg;
        
        Flow("Ability.Attack.Basic")
            .Log("BasicAttack: 공격 시작")
            
            // 타겟 로드 (IntentEvent에서)
            .LoadStore(Target, PropertyId::Param0)      // Param0 = 타겟 EntityId
            
            // 공격 애니메이션
            .PlayAnimMontage(Self, "Attack")
            .WaitAnimEnd(Self)
            
            // 공격력 로드
            .LoadStore(R0, PropertyId::AttackPower)
            
            // 피해 적용
            .ApplyDamage(Target, R0)
            .PlayVFXAttached(Target, "/Game/VFX/HitSpark")
            .PlaySound("/Game/Sounds/Hit")
            
            .Log("BasicAttack: 완료")
            .Halt()
            .BuildAndRegister();
    }
    
    /**
     * ================================================================
     * 추가 예제: 회복 스킬 Flow
     * 
     * 자연어로 읽으면:
     * "시전 애니메이션을 재생하고, 자신의 체력을 회복량만큼 회복한다.
     *  체력이 최대치를 넘지 않도록 한다."
     * ================================================================
     */
    inline void RegisterHeal()
    {
        using namespace Reg;
        
        Flow("Ability.Skill.Heal")
            .Log("Heal: 시전 시작")
            
            // 시전 애니메이션
            .PlayAnim(Self, "CastHeal")
            .PlayVFXAttached(Self, "/Game/VFX/HealCast")
            .WaitSeconds(0.8f)
            
            // 현재 체력과 최대 체력 로드
            .LoadStore(R0, PropertyId::Health)
            .LoadStore(R1, PropertyId::MaxHealth)
            
            // 회복량 (Param0에서, 기본 50)
            .LoadStore(R2, PropertyId::Param0)
            .CmpEq(R3, R2, R3)                          // R2 == 0?
            .JumpIfNot(R3, "HasHealAmount")
            .LoadConst(R2, 50)                          // 기본값 50
            .Label("HasHealAmount")
            
            // 새 체력 = 현재 + 회복량
            .Add(R0, R0, R2)
            
            // 최대 체력 제한
            .CmpGt(R3, R0, R1)                          // 새 체력 > 최대?
            .JumpIfNot(R3, "NoClamp")
            .Move(R0, R1)                               // 최대로 제한
            .Label("NoClamp")
            
            // 체력 저장
            .SaveStore(PropertyId::Health, R0)
            
            // 회복 이펙트
            .PlayVFXAttached(Self, "/Game/VFX/HealBurst")
            .PlaySound("/Game/Sounds/Heal")
            
            .Log("Heal: 완료")
            .Halt()
            .BuildAndRegister();
    }
    
    /** 모든 기본 Flow 등록 */
    inline void RegisterAllFlows()
    {
        RegisterFireball();
        RegisterMoveTo();
        RegisterCharacterSpawn();
        RegisterBasicAttack();
        RegisterHeal();
    }
}