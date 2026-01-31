#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace HktProtoTags
{
    // Flow IDs - HktFlowDefinitions.h에서 사용되는 Flow 식별자
    HKTPROTO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_Fireball);
    HKTPROTO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Move_ToLocation);
    HKTPROTO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Character_Spawn);
    HKTPROTO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_Basic);
    HKTPROTO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_Heal);

    // Effect Tags
    HKTPROTO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Burn);
}
