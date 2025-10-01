#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HktDataAsset.generated.h"

class UHktViewHandle;
class UNiagaraSystem;
class USoundBase;
class UUserWidget;

/**
 * 스폰될 뷰 리소스를 정의하는 추상 베이스 데이터 에셋입니다.
 * 각 리소스 타입은 이 클래스를 상속받아 CreateViewHandle 함수를 구현해야 합니다.
 */
UCLASS(Abstract, BlueprintType, meta = (DisplayName = "Hkt View Data Asset (Base)"))
class HKTCLIENT_API UHktDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    /**
     * 이 데이터 에셋으로 스폰된 뷰 리소스들을 식별하기 위한 고유 태그입니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hkt View Spawning")
    FName SpawnIdentifierTag;

    /**
     * 이 데이터 에셋에 정의된 뷰 리소스를 생성하고, 이를 제어할 핸들을 반환합니다.
     * @param WorldContextObject 스폰이 일어날 월드를 제공하는 오브젝트입니다.
     * @param SpawnTransform 뷰 리소스가 스폰될 월드 트랜스폼입니다.
     * @return 생성된 뷰를 제어하는 핸들, 실패 시 nullptr를 반환합니다.
     */
    virtual UHktViewHandle* CreateViewHandle(UObject* WorldContextObject, const FTransform& SpawnTransform) const PURE_VIRTUAL(UHktDataAsset::CreateViewHandle, return nullptr;);
};

/** Actor 뷰 리소스를 정의하는 데이터 에셋입니다. */
UCLASS(BlueprintType, meta = (DisplayName = "Hkt View Data Asset (Actor)"))
class HKTCLIENT_API UHktActorDataAsset : public UHktDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
    TSubclassOf<AActor> ActorClass;

    virtual UHktViewHandle* CreateViewHandle(UObject* WorldContextObject, const FTransform& SpawnTransform) const override;
};

/** Niagara 뷰 리소스를 정의하는 데이터 에셋입니다. */
UCLASS(BlueprintType, meta = (DisplayName = "Hkt View Data Asset (Niagara)"))
class HKTCLIENT_API UHktNiagaraDataAsset : public UHktDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
    TObjectPtr<UNiagaraSystem> NiagaraSystem;

    virtual UHktViewHandle* CreateViewHandle(UObject* WorldContextObject, const FTransform& SpawnTransform) const override;
};

/** Sound 뷰 리소스를 정의하는 데이터 에셋입니다. */
UCLASS(BlueprintType, meta = (DisplayName = "Hkt View Data Asset (Sound)"))
class HKTCLIENT_API UHktSoundDataAsset : public UHktDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
    TObjectPtr<USoundBase> SoundAsset;

    virtual UHktViewHandle* CreateViewHandle(UObject* WorldContextObject, const FTransform& SpawnTransform) const override;
};

/** Widget 뷰 리소스를 정의하는 데이터 에셋입니다. */
UCLASS(BlueprintType, meta = (DisplayName = "Hkt View Data Asset (Widget)"))
class HKTCLIENT_API UHktWidgetDataAsset : public UHktDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
    TSubclassOf<UUserWidget> WidgetClass;

    virtual UHktViewHandle* CreateViewHandle(UObject* WorldContextObject, const FTransform& SpawnTransform) const override;
};

