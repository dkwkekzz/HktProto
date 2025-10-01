#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HktDef.h"
#include "HktClientViewSubsystem.generated.h"

class UHktViewHandle;
class IHktBehavior;

/**
 * HktDataAsset�� �����Ͽ� ���忡 �پ��� �� ���ҽ�(����, ��ƼŬ ��)�� �����ϰ� �����ϴ� ����ý����Դϴ�.
 */
UCLASS()
class HKTCLIENT_API UHktClientViewSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

protected:
    // UWorldSubsystem overloads
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:
    // --- �̺�Ʈ �ڵ鷯 ---
    void HandleBehaviorCreated(const IHktBehavior& InBehavior);
    void HandleBehaviorDestroyed(const IHktBehavior& InBehavior);
    void HandleSubjectDestroyed(FHktId SubjectId);

    /** ���������� �並 �ı��ϰ� ��� �ʿ��� �����ϴ� ���� �Լ��Դϴ�. */
    void DestroyViewInternal(UHktViewHandle* ViewHandle);

    /**
     * Subject ID -> Behavior ID -> View Handles �迭���� ��� ���Դϴ�.
     * Ư�� Subject�� Behavior�� ���� ��� �並 ������ ã�� ���� ���˴ϴ�.
     */
    UPROPERTY()
    TMap<FHktId, TMap<FHktId, TArray<TObjectPtr<UHktViewHandle>>>> LookupObjToView;

    /**
     * View Handle -> {Subject ID, Behavior ID} ���� ������ ��� ���Դϴ�.
     * Ư�� �䰡 � ��ü�� �����ִ��� ã�ų�, ���� �� �ı� �� ObjToView ���� �����ϱ� ���� ���˴ϴ�.
     * Ű�� TWeakObjectPtr�� ����Ͽ�, �ڵ��� �ٸ� ������ �ı��Ǿ��� �� ���� ��ȿ���� ���� �����͸� ��� ���� �ʵ��� �մϴ�.
     */
    UPROPERTY()
    TMap<TWeakObjectPtr<UHktViewHandle>, TTuple<FHktId, FHktId>> LookupViewToObj;
};

