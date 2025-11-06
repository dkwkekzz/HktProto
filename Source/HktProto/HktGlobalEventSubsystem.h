// HktGlobalEventSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HktGlobalEventSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnUnitSpawnRequestSignature, TSubclassOf<APawn>/* UnitClass*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMinimapCameraMoveRequestSignature, const FVector2D& /* NormalizedLocation */);

/**
 * UI와 게임 로직 간의 글로벌 델리게이트를 관리하는 월드 서브시스템입니다.
 * UI 등 각종 시스템은 이 서브시스템을 통해 '유닛 생성을 요청'합니다.
 */
UCLASS()
class HKTPROTO_API UHktGlobalEventSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * UI(위젯) 등에서 유닛 생성 버튼 클릭 시 호출(Broadcast)할 델리게이트입니다.
	 */
	FOnUnitSpawnRequestSignature OnUnitSpawnRequest;
	FOnMinimapCameraMoveRequestSignature OnMinimapCameraMoveRequest;
};
