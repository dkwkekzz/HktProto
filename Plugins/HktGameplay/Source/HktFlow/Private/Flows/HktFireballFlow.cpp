// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "Flows/HktFireballBehavior.h"
#include "HktJobBuilder.h"

void UHktFireballBehavior::DefineFlow(FHktJobBuilder& Builder, const FHktIntentEvent& Event)
{
	const int32 Caster = Event.Subject.Value;

	// Capture config values for lambda
	const float Speed = FireballSpeed;
	const float Radius = ExplosionRadius;
	const float DirectDmg = DirectDamage;
	const float SplashDmg = SplashDamage;
	const float BurnDuration = BurningDuration;

	/*
	 * ========================================
	 * 파이어볼 전체 흐름 (시간 개념 포함)
	 * ========================================
	 * 
	 * [0.0s] 시전 애니메이션 시작
	 * [~1.0s] 애니메이션 완료 → 파이어볼 생성
	 * [1.0s~] 파이어볼이 앞으로 날아감
	 * [??.?s] 충돌 발생!
	 *         ├─ 파이어볼 파괴
	 *         ├─ 폭발 이펙트 생성
	 *         ├─ 충돌 대상에게 직격 데미지
	 *         └─ 범위 내 모든 대상에게:
	 *            ├─ 스플래시 데미지
	 *            └─ 10초간 Burning 디버프
	 */

	 // -------------------------------------------------------
    // 1. 심볼(Symbol) 정의 
    // -------------------------------------------------------
    // 'Caster'는 EventProvider가 넣어준 기본 컨텍스트라고 가정
    const FName Caster = FName("Caster"); 
    
    // 앞으로 바인딩할 컨텍스트 이름들 (변수명 선언과 유사)
    const FName Ctx_Fireball = FName("Ctx_FireballActor");
    const FName Ctx_HitActor = FName("Ctx_HitActor");
    const FName Ctx_ExplosionTarget = FName("Ctx_LoopTarget");
    
    // 태그 및 상수
    const FGameplayTag Tag_CastingAnim = FGameplayTag::RequestGameplayTag(FName("Anim.Mage.FireballCast"));
    const FGameplayTag Signal_AnimEnd = FGameplayTag::RequestGameplayTag(FName("Event.Anim.End"));
    
    const FGameplayTag Tag_FireballSpawn = FGameplayTag::RequestGameplayTag(FName("Entity.Projectile.Fireball"));
    const FGameplayTag Tag_ExplosionEffect = FGameplayTag::RequestGameplayTag(FName("FX.Explosion.Fire"));
    
    // -------------------------------------------------------
    // 2. 흐름 정의 (Context Binding 적용)
    // -------------------------------------------------------

    // [Step 1] 애니메이션 재생
    FlowBuilder.PlayAnimation(Caster, Tag_CastingAnim)
    
    // [Step 2] 애니메이션 종료 시그널 대기 (OnWait)
    .OnWait(Caster, Signal_AnimEnd)
    [
        // [Step 3] 파이어볼 생성 요청
        FlowBuilder.SpawnEntity(Tag_FireballSpawn)
        
        // [Step 4] 생성된 파이어볼에 대한 처리
        // -> 여기서 "생성된 녀석"을 'Ctx_Fireball'이라는 이름으로 바인딩함
        .OnSpawn(Ctx_Fireball)
        [
            // 이제 Ctx_Fireball을 주체로 사용하여 이동
            FlowBuilder.MoveForward(Ctx_Fireball, 1500.0f)
            
            // [Step 5] 충돌 감지 (직격)
            // -> 충돌한 대상을 'Ctx_HitActor'로 바인딩함
            .OnCollision(Ctx_Fireball, 50.0f, Ctx_HitActor)
            [
                // 직격 데미지 처리
                FlowBuilder.DestroyEntity(Ctx_Fireball)
                           .SetDamage(Ctx_HitActor, 100.0f)
                           .SpawnEffect(Tag_ExplosionEffect)
                
                // [Step 6] 폭발 스플래시 (범위 충돌)
                // -> 범위 내 대상들을 리스트로 가져옴 (내부적으로 임시 그룹 생성)
                .OnCollision(Ctx_Fireball, 300.0f, NAME_None) // 단순 트리거용 범위 체크
                [
                    // 감지된 그룹(ExplosionTargets)을 순회하며 개별 대상을 'Ctx_ExplosionTarget'으로 바인딩
                    FlowBuilder.ForEachTarget(FName("ExplosionTargets"), Ctx_ExplosionTarget)
                    [
                        FlowBuilder.SetDamage(Ctx_ExplosionTarget, 50.0f)
                    ]
                ]
            ]

            // [Step 7] 타임아웃 처리
            .OnWait(3.0f) // 3초 대기
            [
                FlowBuilder.DestroyEntity(Ctx_Fireball)
            ]
        ]
    ];
}

