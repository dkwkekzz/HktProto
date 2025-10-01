#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HktViewHandle.generated.h"

class AActor;
class UNiagaraComponent;
class UAudioComponent;
class UUserWidget;

/**
 * 월드에 스폰된 뷰 리소스를 나타내는 추상 베이스 클래스입니다.
 * 모든 핸들은 자신에게 할당된 뷰를 파괴하는 방법을 알고 있어야 합니다.
 */
UCLASS(Abstract, BlueprintType)
class HKTCLIENT_API UHktViewHandle : public UObject
{
    GENERATED_BODY()

public:
    /** 이 핸들이 관리하는 뷰 리소스를 파괴합니다. */
    UFUNCTION(BlueprintCallable, Category = "Hkt View Handle")
    virtual void DestroyView() PURE_VIRTUAL(UHktViewHandle::DestroyView, );

    /** 이 뷰를 생성한 데이터 에셋의 식별 태그입니다. */
    UPROPERTY(BlueprintReadOnly, Category = "Hkt View Handle")
    FName IdentifierTag;
};

/** Actor 뷰를 위한 핸들입니다. */
UCLASS()
class HKTCLIENT_API UHktActorViewHandle : public UHktViewHandle
{
    GENERATED_BODY()
public:
    void Initialize(AActor* InActor);
    virtual void DestroyView() override;
private:
    TWeakObjectPtr<AActor> ManagedActor;
};

/** Niagara Particle System 뷰를 위한 핸들입니다. */
UCLASS()
class HKTCLIENT_API UHktNiagaraViewHandle : public UHktViewHandle
{
    GENERATED_BODY()
public:
    void Initialize(UNiagaraComponent* InComponent);
    virtual void DestroyView() override;
private:
    TWeakObjectPtr<UNiagaraComponent> ManagedComponent;
};

/** Sound 뷰를 위한 핸들입니다. */
UCLASS()
class HKTCLIENT_API UHktSoundViewHandle : public UHktViewHandle
{
    GENERATED_BODY()
public:
    void Initialize(UAudioComponent* InComponent);
    virtual void DestroyView() override;
private:
    TWeakObjectPtr<UAudioComponent> ManagedComponent;
};

/** User Widget 뷰를 위한 핸들입니다. */
UCLASS()
class HKTCLIENT_API UHktWidgetViewHandle : public UHktViewHandle
{
    GENERATED_BODY()
public:
    void Initialize(UUserWidget* InWidget);
    virtual void DestroyView() override;
private:
    TWeakObjectPtr<UUserWidget> ManagedWidget;
};
