#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HktDataAsset.generated.h"

class UHktViewHandle;
class UNiagaraSystem;
class USoundBase;
class UUserWidget;

/**
 * ������ �� ���ҽ��� �����ϴ� �߻� ���̽� ������ �����Դϴ�.
 * �� ���ҽ� Ÿ���� �� Ŭ������ ��ӹ޾� CreateViewHandle �Լ��� �����ؾ� �մϴ�.
 */
UCLASS(Abstract, BlueprintType, meta = (DisplayName = "Hkt View Data Asset (Base)"))
class HKTCLIENT_API UHktDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    /**
     * �� ������ �������� ������ �� ���ҽ����� �ĺ��ϱ� ���� ���� �±��Դϴ�.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hkt View Spawning")
    FName SpawnIdentifierTag;

    /**
     * �� ������ ���¿� ���ǵ� �� ���ҽ��� �����ϰ�, �̸� ������ �ڵ��� ��ȯ�մϴ�.
     * @param WorldContextObject ������ �Ͼ ���带 �����ϴ� ������Ʈ�Դϴ�.
     * @param SpawnTransform �� ���ҽ��� ������ ���� Ʈ�������Դϴ�.
     * @return ������ �並 �����ϴ� �ڵ�, ���� �� nullptr�� ��ȯ�մϴ�.
     */
    virtual UHktViewHandle* CreateViewHandle(UObject* WorldContextObject, const FTransform& SpawnTransform) const PURE_VIRTUAL(UHktDataAsset::CreateViewHandle, return nullptr;);
};

/** Actor �� ���ҽ��� �����ϴ� ������ �����Դϴ�. */
UCLASS(BlueprintType, meta = (DisplayName = "Hkt View Data Asset (Actor)"))
class HKTCLIENT_API UHktActorDataAsset : public UHktDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
    TSubclassOf<AActor> ActorClass;

    virtual UHktViewHandle* CreateViewHandle(UObject* WorldContextObject, const FTransform& SpawnTransform) const override;
};

/** Niagara �� ���ҽ��� �����ϴ� ������ �����Դϴ�. */
UCLASS(BlueprintType, meta = (DisplayName = "Hkt View Data Asset (Niagara)"))
class HKTCLIENT_API UHktNiagaraDataAsset : public UHktDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
    TObjectPtr<UNiagaraSystem> NiagaraSystem;

    virtual UHktViewHandle* CreateViewHandle(UObject* WorldContextObject, const FTransform& SpawnTransform) const override;
};

/** Sound �� ���ҽ��� �����ϴ� ������ �����Դϴ�. */
UCLASS(BlueprintType, meta = (DisplayName = "Hkt View Data Asset (Sound)"))
class HKTCLIENT_API UHktSoundDataAsset : public UHktDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
    TObjectPtr<USoundBase> SoundAsset;

    virtual UHktViewHandle* CreateViewHandle(UObject* WorldContextObject, const FTransform& SpawnTransform) const override;
};

/** Widget �� ���ҽ��� �����ϴ� ������ �����Դϴ�. */
UCLASS(BlueprintType, meta = (DisplayName = "Hkt View Data Asset (Widget)"))
class HKTCLIENT_API UHktWidgetDataAsset : public UHktDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
    TSubclassOf<UUserWidget> WidgetClass;

    virtual UHktViewHandle* CreateViewHandle(UObject* WorldContextObject, const FTransform& SpawnTransform) const override;
};

