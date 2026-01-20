// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "HktIntentInterface.h"
#include "HktIntentEventComponent.generated.h"

class UHktIntentBuilderComponent;

struct FHktEventContainer;
class UHktIntentEventComponent;
class UHktIntentSubsystem;

UENUM(BlueprintType)
enum class EIntentChangeType : uint8
{
	Added,
	Updated,
	Removed
};

/**
 * [Data Packet]
 * Wraps the Intent Event for FastArraySerializer replication.
 */
USTRUCT(BlueprintType)
struct FHktEventItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FHktEventItem()
	{}

	FHktEventItem(const FHktIntentEvent& InEvent)
		: Event(InEvent)
	{}

	UPROPERTY()
	FHktIntentEvent Event;

	// FAS Callbacks
	void PostReplicatedAdd(const FHktEventContainer& InArraySerializer);
	void PostReplicatedChange(const FHktEventContainer& InArraySerializer);
	void PreReplicatedRemove(const FHktEventContainer& InArraySerializer);
};

/**
 * [Data Container]
 * Synchronizes list of events between Client and Server.
 */
USTRUCT(BlueprintType)
struct FHktEventContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FHktEventItem> Items;

	/** Channel ID - 함께 동기화가 필요한 묶음 식별자 (GameMode에서 제공) */
	UPROPERTY(NotReplicated)
	int32 ChannelId = 0;

	/** Subsystem reference for FAS callbacks */
	UPROPERTY(NotReplicated)
	TObjectPtr<UHktIntentSubsystem> OwnerSubsystem;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FHktEventItem, FHktEventContainer>(Items, DeltaParms, *this);
	}

	// Helper
	FHktEventItem& AddOrUpdateItem(const FHktEventItem& Item);
};

// FAS Traits
template<>
struct TStructOpsTypeTraits<FHktEventContainer> : public TStructOpsTypeTraitsBase2<FHktEventContainer>
{
	enum { WithNetDeltaSerializer = true };
};

/**
 * 클라이언트의 행동 의도(Intent)를 서버로 전송하고, 
 * 서버에서 확정된 이벤트를 다시 클라이언트로 복제하여 동기화하는 컴포넌트
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HKTINTENT_API UHktIntentEventComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHktIntentEventComponent();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- Public Interface ---

	/** 클라이언트에서 입력을 받아 의도 제출 */
	UFUNCTION(BlueprintCallable, Category = "Hkt|Intent")
	void SubmitIntent(UHktIntentBuilderComponent* Builder);

	/** 복제된 이벤트 버퍼 직접 접근 (읽기 전용) */
	UFUNCTION(BlueprintCallable, Category = "Hkt|Intent")
	const TArray<FHktEventItem>& GetEventBuffer() const { return EventBuffer.Items; }

	/** 처리된 이벤트 제거 (Server only, Commit에서 호출) */
	void RemoveProcessedEvents(int32 LastProcessedEventId);

protected:
	// --- Networking ---
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ReceiveEvent(FHktIntentEvent PendingEvent);

private:
	/** 복제된 이벤트 데이터 버퍼 (Fast Array Serializer) */
	UPROPERTY(Replicated)
	FHktEventContainer EventBuffer;

	/** 로컬에서 생성된 의도 ID 시퀀스 (Unique ID 발급용) */
	int32 LocalIntentSequence;
};
