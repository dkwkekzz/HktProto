#pragma once

#include "HktDef.h"


class IHktBehavior
{
public:
	virtual ~IHktBehavior() {}
	virtual uint32 GetTypeId() const = 0;
	virtual FHktId GetSubjectId() const = 0;
	virtual FHktId GetBehaviorId() const = 0;
	virtual FGameplayTagContainer GetTags() const = 0;
};


/**
 * @brief Behavior Ÿ�Ժ��� ���� ID�� �����ϴ� Ŭ����
 */
class FBehaviorTypeIdGenerator
{
public:
    static inline uint32 GetNextId()
    {
        // 0�� ��ȿ���� ���� ������ ���� �� �����Ƿ� 1���� �����մϴ�.
        static uint32 Counter = 1;
        return Counter++;
    }
};

/**
 * @brief ���ø��� ����Ͽ� �� Behavior ������ Ÿ�Կ� ���� ���� ID�� ������ Ÿ�ӿ� �Ҵ��մϴ�.
 * @tparam T USTRUCT�� ���ǵ� ������ Ÿ��
 * @return �ش� Ÿ���� ���� ID
 */
template<typename T>
inline uint32 GetBehaviorTypeId()
{
    static const uint32 Id = FBehaviorTypeIdGenerator::GetNextId();
    return Id;
}

/**
 * @brief ���ŵ� �����͸� ������� Behavior �ν��Ͻ��� �����ϴ� ���丮�Դϴ�.
 */
class FHktBehaviorFactory
{
public:
    // ������ �Լ� Ÿ�� ���� (USTRUCT �����͸� �޾� Behavior�� ����)
    using FBehaviorCreator = TFunction<TUniquePtr<IHktBehavior>()>;

    static inline void Register(uint32 InTypeId, FBehaviorCreator InCreator)
    {
        GetRegistry().Add(InTypeId, InCreator);
    }

    static inline TUniquePtr<IHktBehavior> Create(uint32 InTypeId)
    {
        const FBehaviorCreator* Creator = GetRegistry().Find(InTypeId);
        return Creator ? (*Creator)() : nullptr;
    }

private:
    static inline TMap<uint32, FBehaviorCreator>& GetRegistry()
    {
        static TMap<uint32, FBehaviorCreator> Registry;
        return Registry;
    }
};


/**
 * @brief ������(USTRUCT)�� ����(Execute)�� �����ϴ� CRTP ��� ���ø� Ŭ�����Դϴ�.
 * @tparam Derived �Ļ��� Behavior Ŭ���� Ÿ�� (��: FHktPlayerBehavior)
 * @tparam DataType �� Behavior�� ����� USTRUCT ������ Ÿ�� (��: FHktPlayerData)
 */
template<typename TraitType>
class THktBehavior : public IHktBehavior
{
public:
    using PacketType = typename TraitType::Packet;

    // ���� �� ������ ����ü�� ������ ����(move)�� ���� �޾� ����� �����մϴ�.
    explicit THktBehavior(typename TraitType::Packet&& InData) : Data(MoveTemp(InData)) {}
    virtual ~THktBehavior() override = default;

    // ���ø� ��� ID �����⸦ ����Ͽ� ���� ID�� �ڵ����� �ο��մϴ�.
    virtual uint32 GetTypeId() const
    {
        return GetTypeIdStatic();
    }

private:
    inline static uint32 GetTypeIdStatic()
    {
        return static_cast<uint32>(TraitType::CaseEnum);
    }

    /**
     * @brief �ڵ� ����� ���� ���� ����ü�Դϴ�.
     * �� ����ü�� static �ν��Ͻ��� ������ �� �����ڰ� ȣ��Ǿ� ���丮�� Behavior�� �ڵ����� ����մϴ�.
     */
    struct FAutoRegisterer
    {
        FAutoRegisterer()
        {
            FHktBehaviorFactory::Register(GetTypeIdStatic(), [](typename TraitType::Packet&& InData) -> TUniquePtr<IHktBehavior>
                {
                    return MakeUnique<THktBehavior<TraitType>>(std::move(InData));
                });
        }
    };

    // static �ν��Ͻ��� �����Ͽ� FAutoRegisterer�� �����ڸ� ȣ���ϵ��� �մϴ�.
    static FAutoRegisterer AutoRegisterer;

protected:
    // �Ļ� Ŭ������ �����Ϳ� ������ �� �ֵ��� protected ����� �����մϴ�.
    typename TraitType::Packet Data;
};

// static ��� ������ Ŭ���� �ܺο��� �����մϴ�.
template<typename TraitType>
typename THktBehavior<TraitType>::FAutoRegisterer THktBehavior<TraitType>::AutoRegisterer;
