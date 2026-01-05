#include "HktIntentPlayerState.h"
#include "HktIntentTags.h"
#include "Objects/HktInputContexts.h"

//-----------------------------------------------------------------------------
// Subject Context Implementation for PlayerState
//-----------------------------------------------------------------------------
class FHktPlayerStateSubjectContext : public IHktSubjectContext
{
public:
	explicit FHktPlayerStateSubjectContext(const FHktUnitHandle& InPlayerHandle)
		: PlayerHandleRef(InPlayerHandle)
	{
	}

	virtual TArray<FHktUnitHandle> ResolveSubjects() const override
	{
		return TArray<FHktUnitHandle>({ PlayerHandleRef });
	}

	virtual FHktUnitHandle ResolvePrimarySubject() const override
	{
		return PlayerHandleRef;
	}

	virtual bool IsValid() const override
	{
		return PlayerHandleRef.IsValid();
	}

	virtual bool IsPrimarySubject() const override
	{
		// PlayerState의 Subject는 항상 Primary (자기 자신)
		return true;
	}

private:
	const FHktUnitHandle& PlayerHandleRef;
};

//-----------------------------------------------------------------------------
// Command Context Implementation for PlayerState
//-----------------------------------------------------------------------------
class FHktPlayerStateCommandContext : public IHktCommandContext
{
public:
	explicit FHktPlayerStateCommandContext(const FGameplayTag& InEventTag)
		: EventTagRef(InEventTag)
	{
	}

	virtual FGameplayTag ResolveEventTag() const override
	{
		// 플레이어가 캐릭터를 스폰하기 위한 이벤트 태그 반환
		// 서버로부터 받을 정보(소환할 캐릭터 어셋 정보)는 EventTagRef에 저장됨
		return EventTagRef;
	}

	virtual bool IsValid() const override
	{
		return EventTagRef.IsValid();
	}

	virtual bool IsRequiredTarget() const override
	{
		// PlayerState의 Command는 타겟 불필요 (자기 자신에 대한 명령)
		return false;
	}

private:
	const FGameplayTag& EventTagRef;
};

//-----------------------------------------------------------------------------
// AHktIntentPlayerState
//-----------------------------------------------------------------------------
AHktIntentPlayerState::AHktIntentPlayerState()
{
}

AHktIntentPlayerState::~AHktIntentPlayerState()
{
	SubjectContextImpl.Reset();
	CommandContextImpl.Reset();
}

void AHktIntentPlayerState::BeginPlay()
{
	Super::BeginPlay();

	// Create interface implementations with references to our data
	SubjectContextImpl = MakeUnique<FHktPlayerStateSubjectContext>(PlayerHandle);
	CommandContextImpl = MakeUnique<FHktPlayerStateCommandContext>(PlayerInitialEventTag);
}

IHktSubjectContext* AHktIntentPlayerState::GetSubjectContext() const
{
	return SubjectContextImpl.Get();
}

IHktCommandContext* AHktIntentPlayerState::GetCommandContext() const
{
	return CommandContextImpl.Get();
}