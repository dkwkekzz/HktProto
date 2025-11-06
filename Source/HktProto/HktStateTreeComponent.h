
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "HktStateTreeComponent.generated.h"

// 델리게이트 시그니처 선언
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnStateTagChanged, FGameplayTag, bool);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HKTPROTO_API UHktStateTreeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHktStateTreeComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 유틸리티 함수: 현재 태그가 있는지 확인
	bool HasStateTag(FGameplayTag Tag) const;

	// 1. [핵심] 외부에서 호출할 상태 변경 RPC
	UFUNCTION(Server, Reliable)
	void Server_AddStateTag(FGameplayTag TagToAdd);
	UFUNCTION(Server, Reliable)
	void Server_RemoveStateTag(FGameplayTag TagToRemove);
	// 4. [핵심] 태그 복제 시 클라이언트에서 호출될 함수
	UFUNCTION()
	void OnRep_ActiveStateTags(FGameplayTagContainer OldTags);

	// 2. [핵심] 외부에서 바인딩할 이벤트 델리게이트
	FOnStateTagChanged OnStateTagChanged;

protected:
	// 3. [핵심] 실제 태그 컨테이너
	UPROPERTY(ReplicatedUsing = OnRep_ActiveStateTags)
	FGameplayTagContainer ActiveStateTags;
	
	UPROPERTY(EditAnywhere, Category="Debug")
	bool bEnableDebugDrawing = false;
};
