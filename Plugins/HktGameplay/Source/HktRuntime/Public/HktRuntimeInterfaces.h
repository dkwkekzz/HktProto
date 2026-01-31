// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HktRuntimeTypes.h"
#include "HktCoreInterfaces.h"
#include "HktRuntimeInterfaces.generated.h"

class AHktPlayerController;

//=============================================================================
// IHktSelectable - 선택 가능한 오브젝트 인터페이스
//=============================================================================

UINTERFACE(MinimalAPI, BlueprintType)
class UHktSelectable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 선택 가능한 오브젝트가 구현하는 인터페이스.
 * EntityId를 제공하여 Intent 시스템에서 Subject/Target으로 사용됨.
 */
class HKTRUNTIME_API IHktSelectable
{
	GENERATED_BODY()

public:
	/** 이 오브젝트의 EntityId를 반환 */
	virtual FHktEntityId GetEntityId() const = 0;
	
	/** 선택 가능 여부 반환 (기본: true) */
	virtual bool IsSelectable() const { return true; }
};

/**
 * IHktRelevancyProvider - Relevancy 관리 인터페이스
 *
 * 클라이언트별로 어떤 이벤트를 전송할지 결정하는 정책 인터페이스
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UHktRelevancyProvider : public UInterface
{
	GENERATED_BODY()
};

class HKTRUNTIME_API IHktRelevancyProvider
{
	GENERATED_BODY()

public:
    /** 클라이언트 등록 */
    virtual void RegisterClient(AHktPlayerController* Client) = 0;
    
    /** 클라이언트 등록 해제 */
    virtual void UnregisterClient(AHktPlayerController* Client) = 0;
    
    /** 모든 등록된 클라이언트 목록 */
    virtual const TArray<AHktPlayerController*>& GetAllClients() const = 0;
    
    /** 특정 위치에 관심 있는 클라이언트들 조회 */
    virtual void GetRelevantClientsAtLocation(
        const FVector& Location,
        TArray<AHktPlayerController*>& OutRelevantClients
    ) = 0;
    
    /** 모든 클라이언트 조회 (글로벌 이벤트용) */
    virtual void GetAllRelevantClients(TArray<AHktPlayerController*>& OutRelevantClients) = 0;
    
    /** Relevancy 업데이트 (매 프레임 호출) */
    virtual void UpdateRelevancy() = 0;
};

// ============================================================================
// 델리게이트 선언
// ============================================================================

DECLARE_MULTICAST_DELEGATE_OneParam(FOnHktSubjectChanged, FHktEntityId);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHktTargetChanged, FHktEntityId);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHktCommandChanged, FGameplayTag);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHktIntentSubmitted, const FHktIntentEvent&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHktWheelInput, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHktEntityCreated, FHktEntityId);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHktEntityDestroyed, FHktEntityId);

// ============================================================================
// IHktModelProvider 인터페이스
// ============================================================================

UINTERFACE(MinimalAPI, BlueprintType)
class UHktModelProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * IHktModelProvider
 * 
 */
class HKTRUNTIME_API IHktModelProvider
{
	GENERATED_BODY()

public:
	// ========== Stash 접근 ==========
	
	/** 엔티티 데이터 읽기용 Stash 인터페이스 */
	virtual IHktStashInterface* GetStashInterface() const = 0;

	// ========== Intent Builder 상태 ==========
	
	/** 현재 선택된 Subject EntityId */
	virtual FHktEntityId GetSelectedSubject() const = 0;
	
	/** 현재 선택된 Target EntityId */
	virtual FHktEntityId GetSelectedTarget() const = 0;
	
	/** 현재 타겟 위치 (Ground 클릭 등) */
	virtual FVector GetTargetLocation() const = 0;
	
	/** 현재 선택된 커맨드 태그 */
	virtual FGameplayTag GetSelectedCommand() const = 0;
	
	/** Intent 빌더가 유효한 상태인지 */
	virtual bool IsIntentValid() const = 0;

	// ========== 델리게이트 ==========
	
	/** Subject 선택 변경 시 */
	virtual FOnHktSubjectChanged& OnSubjectChanged() = 0;
	
	/** Target 선택 변경 시 */
	virtual FOnHktTargetChanged& OnTargetChanged() = 0;
	
	/** Command 선택 변경 시 */
	virtual FOnHktCommandChanged& OnCommandChanged() = 0;
	
	/** Intent 제출 시 */
	virtual FOnHktIntentSubmitted& OnIntentSubmitted() = 0;
	
	/** 휠 입력 시 (줌용) */
	virtual FOnHktWheelInput& OnWheelInput() = 0;
	
	/** 엔티티 생성 시 (Stash에 추가됨) */
	virtual FOnHktEntityCreated& OnEntityCreated() = 0;
	
	/** 엔티티 파괴 시 (Stash에서 제거됨) */
	virtual FOnHktEntityDestroyed& OnEntityDestroyed() = 0;
};
