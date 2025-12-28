// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "HktInputContexts.h" // Interface definitions
#include "HktIntentComponent.generated.h"

struct FHktIntentContainer;
class UHktIntentComponent;

/**
 * [Data Packet]
 * Wraps the Intent Event for FastArraySerializer replication.
 */
USTRUCT(BlueprintType)
struct FHktIntentItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FHktIntentItem()
	{}

	FHktIntentItem(const FHktIntentEvent& InEvent)
		: Event(InEvent)
	{}

	UPROPERTY()
	FHktIntentEvent Event;

	// FAS Callbacks
	void PostReplicatedAdd(const FHktIntentContainer& InArraySerializer);
	void PostReplicatedChange(const FHktIntentContainer& InArraySerializer);
	void PreReplicatedRemove(const FHktIntentContainer& InArraySerializer);
};

/**
 * [Data Container]
 * Synchronizes list of intents/events between Client and Server.
 */
USTRUCT(BlueprintType)
struct FHktIntentContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FHktIntentItem> Items;

	// Component reference for callbacks
	UPROPERTY(NotReplicated)
	TObjectPtr<UHktIntentComponent> OwnerComponent;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FHktIntentItem, FHktIntentContainer>(Items, DeltaParms, *this);
	}

	// Helper
	FHktIntentItem& AddOrUpdateItem(const FHktIntentItem& Item);
};

// FAS Traits
template<>
struct TStructOpsTypeTraits<FHktIntentContainer> : public TStructOpsTypeTraitsBase2<FHktIntentContainer>
{
	enum { WithNetDeltaSerializer = true };
};

UENUM(BlueprintType)
enum class EIntentChangeType : uint8
{
	Added,
	Updated,
	Removed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnIntentChanged, const FHktIntentEvent&, Event, EIntentChangeType, ChangeType);

/**
 * 클라이언트의 행동 의도(Intent)를 서버로 전송하고, 
 * 서버에서 확정된 의도를 다시 클라이언트로 복제하여 동기화하는 컴포넌트
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HKTINTENT_API UHktIntentComponent : public UActorComponent
{
    GENERATED_BODY()

public:	
    UHktIntentComponent();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // --- Public Interface ---

    /** 클라이언트에서 입력을 받아 의도 제출 */
    UFUNCTION(BlueprintCallable, Category = "Hkt|Intent")
    void SubmitIntent(const TScriptInterface<IHktSubjectContext> Subject, 
                      const TScriptInterface<IHktCommandContext> Command, 
                      const TScriptInterface<IHktTargetContext> Target);

    /** 현재 캐시된 유효 의도 목록 반환 */
    UFUNCTION(BlueprintCallable, Category = "Hkt|Intent")
    const TArray<FHktIntentEvent>& GetIntentEvents() const;

    // --- Delegates ---
    UPROPERTY(BlueprintAssignable, Category = "Hkt|Intent")
    FOnIntentChanged OnIntentChanged;

    // --- Internal / FAS Interface ---
    /** FAS 아이템 변경 시 호출됨 */
    void NotifyIntentChanged(const FHktIntentEvent& Event, EIntentChangeType ChangeType);

protected:
    // --- Networking ---
    
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_ReceiveEvent(FHktIntentEvent PendingEvent);

private:
    // --- Properties ---

    /** 로컬에서 생성된 의도 ID 시퀀스 (Unique ID 발급용) */
    int32 LocalIntentSequence;

    /** 복제된 의도 데이터 버퍼 (Fast Array Serializer) */
    UPROPERTY(Replicated)
    FHktIntentContainer IntentBuffer;

    /** * IntentBuffer의 내용을 UI나 로직에서 쉽게 접근하기 위해 유지하는 캐시 배열.
     * NotifyIntentChanged를 통해 동기화됨.
     */
    UPROPERTY(Transient)
    TArray<FHktIntentEvent> CachedIntentEvents;
};