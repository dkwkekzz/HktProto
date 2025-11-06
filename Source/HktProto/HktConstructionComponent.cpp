// HktConstructionComponent.cpp

#include "HktConstructionComponent.h"
#include "HktGlobalEventSubsystem.h" // 델리게이트 서브시스템 포함 (이름 변경)
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"

UHktConstructionComponent::UHktConstructionComponent()
{
	// 이 컴포넌트가 네트워크상에 존재하고 RPC를 수신할 수 있도록 설정
	SetIsReplicatedByDefault(true);
}

void UHktConstructionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// 이 컴포넌트에서 복제할 변수가 있다면 여기에 추가 (DOREPLIFETIME)
}

void UHktConstructionComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// [Client Only]
	// 오직 이 컴포넌트를 소유한 '로컬' 플레이어(클라이언트)만 UI 델리게이트를 구독해야 합니다.
	// 서버나 다른 클라이언트의 프록시는 UI와 상호작용하지 않습니다.
	APlayerController* OwningController = Cast<APlayerController>(GetOwner());
	if (OwningController && OwningController->IsLocalController())
	{
		if (UWorld* World = GetWorld())
		{
			// 변경된 이름의 서브시스템을 가져옵니다.
			if (UHktGlobalEventSubsystem* GlobalEvents = World->GetSubsystem<UHktGlobalEventSubsystem>())
			{
				// GlobalEvents->OnUnitSpawnRequest 델리게이트에 우리의 HandleSpawnRequest 함수를 '구독'
				GlobalEvents->OnUnitSpawnRequest.AddUObject(this, &UHktConstructionComponent::HandleSpawnRequest);
			}
		}
	}
}

void UHktConstructionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 컴포넌트 파괴 시 델리게이트 바인딩 해제 (메모리 누수 방지)
	APlayerController* OwningController = Cast<APlayerController>(GetOwner());
	if (OwningController && OwningController->IsLocalController())
	{
		if (UWorld* World = GetWorld())
		{
			// 변경된 이름의 서브시스템을 가져옵니다.
			if (UHktGlobalEventSubsystem* GlobalEvents = World->GetSubsystem<UHktGlobalEventSubsystem>())
			{
				GlobalEvents->OnUnitSpawnRequest.RemoveAll(this);
			}
		}
	}
	
	Super::EndPlay(EndPlayReason);
}

// [Client] 델리게이트에 의해 호출되는 함수
void UHktConstructionComponent::HandleSpawnRequest(TSubclassOf<APawn> UnitClass)
{
	if (!UnitClass)
	{
		return;
	}

	// TODO: 실제 생성 위치를 결정해야 함 (예: 선택된 건물 위치, 기본 집결지 등)
	// 지금은 임시로 컨트롤러 앞 500cm 위치에 스폰 요청
	APlayerController* OwningController = Cast<APlayerController>(GetOwner());
	if (OwningController)
	{
		FVector Location = OwningController->GetFocalLocation() + OwningController->GetControlRotation().Vector() * 500.0f;
		RequestSpawnUnit(UnitClass, Location);
	}
}

// [Client] 서버로 RPC 전송 시작
void UHktConstructionComponent::RequestSpawnUnit(TSubclassOf<APawn> UnitClass, const FVector& SpawnLocation)
{
	// 서버 RPC 호출
	Server_RequestSpawnUnit(UnitClass, SpawnLocation);
}

// [Validation] 서버 RPC 유효성 검사 (필수)
bool UHktConstructionComponent::Server_RequestSpawnUnit_Validate(TSubclassOf<APawn> UnitClass, const FVector& SpawnLocation)
{
	if (!UnitClass)
	{
		// 클래스가 유효하지 않으면 RPC 자체를 거부 (해킹 방지)
		return false;
	}
	// TODO: 스폰 위치가 맵 밖이 아닌지 등 간단한 추가 검사
	return true;
}

// [Server] 서버에서 실제 실행되는 함수
void UHktConstructionComponent::Server_RequestSpawnUnit_Implementation(TSubclassOf<APawn> UnitClass, const FVector& SpawnLocation)
{
	// 이 코드는 서버에서만 실행됨
	
	// TODO: HktProto의 게임 로직(자원, 인구수) 검사
	// 예: AMyPlayerState* PS = GetOwner<APlayerController>()->GetPlayerState<AMyPlayerState>();
	// if (PS && PS->CanAfford(UnitClass) && PS->HasPopulationSpace())
	
	bool bCanSpawn = true; // 임시로 true

	if (bCanSpawn)
	{
		// TODO: 실제 자원 차감 로직 (PS->SpendResources(UnitClass))
		
		// [Server] 서버에서 액터를 스폰합니다.
		SpawnUnitInternal(UnitClass, SpawnLocation);
	}
	else
	{
		// [Server -> Client] 생성 실패를 클라이언트에게 알림
		Client_NotifySpawnFailed(TEXT("자원 또는 인구수가 부족합니다."));
	}
}

// [Server] 실제 스폰 로직
void UHktConstructionComponent::SpawnUnitInternal(TSubclassOf<APawn> UnitClass, const FVector& SpawnLocation)
{
	if (UWorld* World = GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner(); // 스폰되는 유닛의 Owner를 PlayerController로 설정
		SpawnParams.Instigator = GetOwner<APawn>(); // (만약 폰이 있다면)

		// 서버에서 스폰합니다.
		APawn* NewUnit = World->SpawnActor<APawn>(UnitClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

		if (NewUnit)
		{
			// 스폰 성공. NewUnit의 bReplicates = true라면 자동으로 모든 클라이언트에 복제됩니다.
			UE_LOG(LogTemp, Log, TEXT("Server spawned unit: %s"), *NewUnit->GetName());
		}
	}
}


// [Client] 실패 알림을 받는 함수
void UHktConstructionComponent::Client_NotifySpawnFailed_Implementation(const FString& Reason)
{
	// 이 코드는 요청한 클라이언트에서만 실행됨
	UE_LOG(LogTemp, Warning, TEXT("Spawn Failed: %s"), *Reason);
	
	// TODO: 화면에 "자원 부족" 등 UI 피드백 표시
}

