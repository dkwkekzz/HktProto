#include "HktClientTestsUtil.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

UWorld* HktClientTestsUtil::GetTestWorld()
{
    const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
    for (const FWorldContext& Context : WorldContexts)
    {
        if (Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game)
        {
            return Context.World();
        }
    }
    return nullptr;
}
