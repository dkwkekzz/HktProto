// Fill out your copyright notice in the Description page of Project Settings.


#include "HktRtsUnit.h"
#include "HktStateTreeComponent.h"
#include "Net/UnrealNetwork.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"

AHktRtsUnit::AHktRtsUnit()
{
 	// Set this character to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	// 리플리케이션 활성화
	bReplicates = true;
	SetReplicateMovement(true); // ACharacter는 이동 리플리케이션이 잘 구현되어 있음

	// StateTree 컴포넌트 생성
	StateTreeComponent = CreateDefaultSubobject<UHktStateTreeComponent>(TEXT("StateTreeComponent"));

	// 유닛이 많을 경우를 대비한 최적화 설정
	// (프로토타입 이후 성능 문제가 발생하면 더 세부적인 조정 필요)
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
}

void AHktRtsUnit::BeginPlay()
{
	Super::BeginPlay();
	
	// 서버에서만 AI 컨트롤러를 스폰하고 Possess 하도록 설정
	if (HasAuthority())
	{
		SpawnDefaultController();
	}
}

void AHktRtsUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHktRtsUnit::MoveUnitTo(const FVector& TargetLocation)
{
	// 이 함수는 반드시 서버에서만 호출되어야 합니다.
	if (!HasAuthority())
	{
		return;
	}

	// 유닛에 할당된 AI 컨트롤러 가져오기
	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController)
	{
		// UNavigationSystemV1::SimpleMoveToLocation(AIController, TargetLocation);
		// 또는 더 복잡한 경로 요청
		AIController->MoveToLocation(TargetLocation);

		// TODO: StateTree에 '이동 중' 상태로 변경하라는 이벤트 전달
	}
}

void AHktRtsUnit::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// DOREPLIFETIME(AHktRtsUnit, CurrentState);
	// TODO: 리플리케이트할 상태 변수 등록
}

// void AHktRtsUnit::OnRep_CurrentState()
// {
// 	// 클라이언트에서 상태 변경 시 호출됨
// 	// 예: 상태에 맞는 애니메이션 재생, 이펙트 표시 등
// }
