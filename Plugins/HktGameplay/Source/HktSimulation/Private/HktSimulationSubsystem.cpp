// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktSimulationSubsystem.h"
#include "HktFlowBuilder.h"
#include "HktIntentInterfaces.h"
#include "HktServiceSubsystem.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY(LogHktSimulation);

void UHktSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// HktServiceSubsystem 의존성 초기화
	Collection.InitializeDependency<UHktServiceSubsystem>();

	UE_LOG(LogHktSimulation, Log, TEXT("HktSimulationSubsystem Initialized"));
}

void UHktSimulationSubsystem::Deinitialize()
{
	ActiveVMs.Empty();
	ExternalToInternalMap.Empty();
	InternalToExternalMap.Empty();
	ChannelCursors.Empty();

	UE_LOG(LogHktSimulation, Log, TEXT("HktSimulationSubsystem Deinitialized"));

	Super::Deinitialize();
}

void UHktSimulationSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. IntentEvent 수집 및 처리 (Sliding Window 방식)
	ProcessIntentEvents(DeltaTime);

	// 2. 활성 VM들 틱
	TickActiveVMs(DeltaTime);

	// 3. 완료된 VM 정리
	CleanupFinishedVMs();
}

TStatId UHktSimulationSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UHktSimulationSubsystem, STATGROUP_Tickables);
}

bool UHktSimulationSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

UHktSimulationSubsystem* UHktSimulationSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}

	return World->GetSubsystem<UHktSimulationSubsystem>();
}

FUnitHandle UHktSimulationSubsystem::GetOrCreateInternalHandle(int32 ExternalUnitId, FVector Location, FRotator Rotation)
{
	// 이미 매핑이 있는지 확인
	if (FUnitHandle* Found = ExternalToInternalMap.Find(ExternalUnitId))
	{
		if (EntityManager.IsUnitValid(*Found))
		{
			return *Found;
		}
		// 무효한 핸들이면 제거하고 새로 생성
		ExternalToInternalMap.Remove(ExternalUnitId);
	}

	// 새 유닛 할당
	FPlayerHandle OwnerPlayer; // TODO: 외부에서 플레이어 정보도 전달받아야 함
	FUnitHandle NewHandle = EntityManager.AllocUnit(OwnerPlayer, Location, Rotation);

	// 매핑 등록
	ExternalToInternalMap.Add(ExternalUnitId, NewHandle);
	InternalToExternalMap.Add(NewHandle.Index, ExternalUnitId);

	return NewHandle;
}

int32 UHktSimulationSubsystem::GetExternalUnitId(FUnitHandle InternalHandle) const
{
	if (const int32* Found = InternalToExternalMap.Find(InternalHandle.Index))
	{
		return *Found;
	}
	return INDEX_NONE;
}

int64 UHktSimulationSubsystem::GetChannelCursor(int32 ChannelId) const
{
	if (const int64* Cursor = ChannelCursors.Find(ChannelId))
	{
		return *Cursor;
	}
	return 0;
}

void UHktSimulationSubsystem::SetChannelCursor(int32 ChannelId, int64 NewCursor)
{
	ChannelCursors.Add(ChannelId, NewCursor);
}

void UHktSimulationSubsystem::ExecuteIntentEvent(const FHktIntentEvent& Event)
{
	// Subject의 내부 핸들 획득 (없으면 생성)
	FUnitHandle SubjectHandle = GetOrCreateInternalHandle(Event.Subject.Value, Event.Location);

	// VM 생성
	TUniquePtr<FHktFlowVM> NewVM = MakeUnique<FHktFlowVM>(GetWorld(), &EntityManager, SubjectHandle);

	// 이벤트에 따른 바이트코드 빌드
	BuildBytecodeForEvent(*NewVM, Event);

	// 바이트코드가 비어있지 않으면 실행 큐에 추가
	if (NewVM->Bytecode.Num() > 0)
	{
		ActiveVMs.Add(MoveTemp(NewVM));
		UE_LOG(LogHktSimulation, Verbose, TEXT("Queued VM for IntentEvent: %s (EventId: %d)"), 
			*Event.EventTag.ToString(), Event.EventId);
	}
}

void UHktSimulationSubsystem::ProcessIntentEvents(float DeltaTime)
{
	// ServiceSubsystem에서 IntentEventProvider 획득
	UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld());
	if (!Service)
	{
		return;
	}

	TScriptInterface<IHktIntentEventProvider> Provider = Service->GetIntentEventProvider();
	if (!Provider)
	{
		return;
	}

	// 기본 채널에서 이벤트 가져오기
	const int32 DefaultChannelId = 0;
	TSharedPtr<IHktIntentChannel> Channel = Provider->GetChannel(DefaultChannelId);
	if (!Channel)
	{
		return;
	}

	// [Sliding Window] 커서 이후의 새 이벤트만 조회
	int64& MyCursor = ChannelCursors.FindOrAdd(DefaultChannelId, 0);
	
	TArray<FHktIntentEventEntry> NewEntries;
	if (!Channel->FetchNewEvents(MyCursor, NewEntries))
	{
		return; // 새 이벤트 없음
	}

	// 이벤트 처리
	for (const FHktIntentEventEntry& Entry : NewEntries)
	{
		// 이벤트 처리 시도
		ExecuteIntentEvent(Entry.EventData);

		// [중요] 처리 성공 시 커서 업데이트
		// 만약 처리 도중 실패하거나 yield 해야 한다면 커서를 업데이트하지 않음으로써
		// 다음 프레임에 다시 시도할 수 있는 기회를 가짐 (재진입성 확보)
		MyCursor = FMath::Max(MyCursor, Entry.SequenceId);
	}

	if (NewEntries.Num() > 0)
	{
		UE_LOG(LogHktSimulation, Verbose, TEXT("Processed %d events, Cursor updated to %lld"), 
			NewEntries.Num(), MyCursor);
	}
}

void UHktSimulationSubsystem::TickActiveVMs(float DeltaTime)
{
	for (auto& VM : ActiveVMs)
	{
		if (VM.IsValid())
		{
			VM->Tick(DeltaTime);
		}
	}
}

void UHktSimulationSubsystem::CleanupFinishedVMs()
{
	ActiveVMs.RemoveAll([](const TUniquePtr<FHktFlowVM>& VM) 
	{
		return !VM.IsValid() || VM->Regs.ProgramCounter >= VM->Bytecode.Num();
	});
}

void UHktSimulationSubsystem::BuildBytecodeForEvent(FHktFlowVM& VM, const FHktIntentEvent& Event)
{
	FHktFlowBuilder B(VM.Bytecode);

	// EventTag에 따라 다른 바이트코드 빌드
	// TODO: Flow 레지스트리와 연동하여 동적으로 바이트코드 생성
	
	FString TagString = Event.EventTag.ToString();

	// 예시: 태그에 따른 기본 처리
	// 실제 구현에서는 Flow 정의 시스템과 연동
	
	if (TagString.Contains(TEXT("Attack")))
	{
		// 공격 이벤트 처리 예시
		// Target이 있으면 Target에게 데미지
		if (Event.Target.IsValid())
		{
			// Target의 내부 핸들 획득
			FUnitHandle TargetHandle = GetOrCreateInternalHandle(Event.Target.Value, FVector::ZeroVector);
			
			// GPR[1]에 타겟 설정
			VM.Regs.Get(1).SetUnit(TargetHandle);
			
			// 데미지 적용 (Magnitude를 데미지로 사용)
			float Damage = Event.Magnitude > 0.0f ? Event.Magnitude : 10.0f;
			B.ModifyHealth(1, -Damage);
		}
	}
	else if (TagString.Contains(TEXT("Move")))
	{
		// 이동 이벤트는 별도 처리 (MoveSpeed 기반 이동 시스템에서 처리)
		// 여기서는 위치 업데이트만 수행
		if (EntityManager.IsUnitValid(VM.Regs.OwnerUnit))
		{
			// 이동 목표 위치를 레지스터에 저장
			VM.Regs.Get(0).SetVector(Event.Location);
		}
	}
	else if (TagString.Contains(TEXT("Skill")))
	{
		// 스킬 이벤트 예시: Fireball
		constexpr uint8 RegProjectile = 0;
		
		B.PlayAnim(FName("Montage_Cast"))
		 .WaitSeconds(0.3f);
		
		// 스킬에 따른 추가 처리
		if (TagString.Contains(TEXT("Fireball")))
		{
			// Fireball 스킬: 투사체 생성 -> 충돌 대기 -> 폭발
			// ProjectileClass는 별도 시스템에서 관리
			// B.SpawnFireball(FireballClass, RegProjectile)
			//  .WaitForImpact(RegProjectile)
			//  .Explode(500.0f, 100.0f, 10.0f, 5.0f, RegProjectile);
		}
	}
	else if (TagString.Contains(TEXT("Spawn")))
	{
		// 스폰 이벤트: 새 유닛 생성은 이미 GetOrCreateInternalHandle에서 처리됨
		// 추가 초기화가 필요하면 여기서 수행
	}
	
	// 바이트코드가 비어있으면 기본 no-op 추가 방지
	// (빈 바이트코드는 VM 실행 큐에 추가하지 않음)
}
