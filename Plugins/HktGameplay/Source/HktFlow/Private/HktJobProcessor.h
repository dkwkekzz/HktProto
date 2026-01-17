#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "System/HktJobBuilder.h" // FHktJobCommand, FHktJobBuilder 참조
#include "HktJobProcessor.generated.h"

/**
 * JobBuilder에 의해 기록된 커맨드를 실제로 실행하는 처리기.
 * 타이머 관리, 액터 스폰, 충돌 감지 로직을 수행합니다.
 */
UCLASS()
class HKTGAME_API UHktJobProcessor : public UObject
{
    GENERATED_BODY()

public:
};