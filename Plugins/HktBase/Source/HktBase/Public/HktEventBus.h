/**
 * @file HktEventBus.h
 * @brief NodeIndex�� GameplayTag�� Key��, TPayload Ÿ�Ժ� �̺�Ʈ�� �����ϴ� ����� ��ü�Դϴ�.
 * @tparam TPayload �̺�Ʈ �߻� �� ������ ������ Ÿ���Դϴ�.
 */

#pragma once

#include "HktDef.h"

 /**
  * @brief NodeIndex�� GameplayTag�� Key��, Ư�� ������ Ÿ��(TPayload)�� �̺�Ʈ�� ������ϴ� �̱��� Ŭ�����Դϴ�.
  * GameplayTag�� ���� ������ Ȱ���Ͽ� �پ��� ����� �̺�Ʈ ���İ� �����մϴ�.
  * @tparam TPayload �̺�Ʈ ���̷ε�� ����� ������ Ÿ���Դϴ�.
  */
template<typename TPayload>
struct THktEventBus
{
public:
    /**
     * @brief TPayload Ÿ���� ���� �ϳ��� �޴� ��Ƽĳ��Ʈ ��������Ʈ�Դϴ�.
     * const&�� ���ڸ� �����Ͽ� ���ʿ��� ���縦 �����մϴ�.
     */
    DECLARE_MULTICAST_DELEGATE_OneParam(FBehaviorEvent, const TPayload& /* Payload */);

    /**
     * @brief Ư�� ��忡 ���� �̺�Ʈ���� GameplayTag�� Ű�� �Ͽ� �����ϴ� ������Դϴ�.
     * TArray ��� TMap�� ����Ͽ� Ư�� �±׿� ���� �̺�Ʈ ������ O(1)�� ������ ����ȭ�մϴ�.
     */
    struct FEventRouter
    {
        TMap<FGameplayTag, FBehaviorEvent> Events;
    };

    /**
     * @brief EventBus�� �̱��� �ν��Ͻ��� ��ȯ�մϴ�.
     * @return THktEventBus�� �̱��� �ν��Ͻ�
     */
    static THktEventBus& Get()
    {
        static THktEventBus Instance;
        return Instance;
    }

    /**
     * @brief Ư�� �±׿� ���� �̺�Ʈ ��������Ʈ�� �����ɴϴ�. �̺�Ʈ ���ε�(Bind) �� ���˴ϴ�.
     * �ش� �±��� �̺�Ʈ�� �������� ������ ���� �����Ͽ� ��ȯ�մϴ�.
     * @param NodeIndex �̺�Ʈ�� �ĺ��ϴ� ����� �ε����Դϴ�.
     * @param Tag ���ε��� �̺�Ʈ�� ��Ȯ�� GameplayTag�Դϴ�.
     * @return �ش� �±׿� ���� FBehaviorEvent ��������Ʈ�� ���۷����Դϴ�.
     */
    FBehaviorEvent& GetEvent(FHktId NodeIndex, const FGameplayTag& Tag)
    {
        // FindOrAdd�� ����Ͽ� ��忡 �ش��ϴ� ����͸� ã�ų� ������ �߰��մϴ�.
        FEventRouter& Router = EventRouterMap.FindOrAdd(NodeIndex);
        // ����� ������ Ư�� �±׿� �ش��ϴ� �̺�Ʈ�� ã�ų� ������ �߰��մϴ�.
        return Router.Events.FindOrAdd(Tag);
    }

    FORCEINLINE FBehaviorEvent& GetEvent(FHktId NodeIndex)
    {
        return GetEvent(NodeIndex, FGameplayTag());
    }

    /**
     * @brief ������ �±׿� '��Ȯ��' ��ġ�ϴ� �����ʿ��Ը� �̺�Ʈ�� ����(Broadcast)�մϴ�.
     * @param NodeIndex �̺�Ʈ�� �߻��� ����� �ε����Դϴ�.
     * @param Tag ������ �̺�Ʈ�� GameplayTag�Դϴ�.
     * @param Payload �̺�Ʈ�� �Բ� ������ �������Դϴ�.
     */
    void Broadcast(FHktId NodeIndex, const FGameplayTag& Tag, const TPayload& Payload)
    {
        // �ش� ��忡 ����Ͱ� �����ϴ��� Ȯ���մϴ�.
        if (FEventRouter* Router = EventRouterMap.Find(NodeIndex))
        {
            // ����� ������ ��Ȯ�� ��ġ�ϴ� �±��� �̺�Ʈ�� ã���ϴ�.
            if (FBehaviorEvent* Event = Router->Events.Find(Tag))
            {
                // ��������Ʈ�� ���ε��� �Լ��� �ִ��� Ȯ�� �� �̺�Ʈ�� �����մϴ�.
                if (Event->IsBound())
                {
                    Event->Broadcast(Payload);
                }
            }
        }
    }

private:
    // private �����ڷ� �̱��� ������ �����մϴ�.
    THktEventBus() = default;
    ~THktEventBus() = default;
    THktEventBus(const THktEventBus&) = delete;
    THktEventBus& operator=(const THktEventBus&) = delete;

    /**
     * @brief NodeIndex�� Ű��, FEventRouter�� ������ ������ ���Դϴ�.
     * �� ���(��: Ư�� ĳ���ͳ� ����)���� �������� �̺�Ʈ ����͸� �����մϴ�.
     */
    TMap<FHktId, FEventRouter> EventRouterMap;
};
