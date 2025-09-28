#include "HktRpcProxy.h"
#include "HktRpcTraits.h"
#include "HAL/RunnableThread.h"
#include "HAL/Runnable.h"
#include "Async/Async.h"

// Unreal Engine�� FRunnable�� ���� �Լ��� �Բ� ����ϱ� ���� ���� Ŭ�����Դϴ�.
class FLambdaRunnable : public FRunnable
{
public:
    // ������ ���� �Լ��� �޴� ������
    FLambdaRunnable(TFunction<void()> InFunction) : Function(MoveTemp(InFunction)) {}

    // FRunnable �������̽� ����
    virtual uint32 Run() override
    {
        if (Function)
        {
            Function(); // ����� ���� �Լ� ����
        }
        return 0;
    }

private:
    TFunction<void()> Function;
};

// 모든 비동기 호출을 CompletionQueue에서 다형적으로 처리하기 위한 기본 인터페이스.
// CompletionQueue의 'tag'로 이 인터페이스를 구현한 객체의 포인터가 사용됩니다.
class IAsyncCall {
public:
    virtual ~IAsyncCall() = default;

    // gRPC 응답이 도착했을 때 CqLoop 스레드에서 호출될 함수입니다.
    virtual void Proceed() = 0;
};

// 단일 요청/응답(Unary) RPC의 비동기 호출 과정을 캡슐화하는 제네릭 클래스입니다.
template <typename TCallTrait>
class TGenericAsyncCall final : public IAsyncCall
{
public:
    using TRequest = typename TCallTrait::TRequest;
    using TResponse = typename TCallTrait::TResponse;
    // RPC 완료 시 실행될 콜백 함수의 타입입니다.
    using CallbackFunc = TFunction<void(const TResponse&, const grpc::Status&)>;

    TGenericAsyncCall(
        hkt::HktService::Stub* InStub,
        grpc::CompletionQueue* InCq,
        const TRequest& InRequest,
        CallbackFunc&& InCallback
    ) : Callback(MoveTemp(InCallback))
    {
        // 1. Stub의 비동기 함수를 호출하여 RPC를 준비합니다.
        ResponseReader = (InStub->*TCallTrait::PrepareFunc)(&Context, InRequest, InCq);
        // 2. RPC 호출을 시작합니다.
        ResponseReader->StartCall();
        // 3. 응답이 오면 CompletionQueue에 알리도록 등록합니다. 이 객체의 주소(this)를 태그로 사용합니다.
        ResponseReader->Finish(&Reply, &Status, this);
    }

    // CqLoop에서 응답을 감지하고 호출하는 함수입니다.
    void Proceed() override
    {
        // 4. 저장해둔 콜백 함수를 실행합니다.
        Callback(Reply, Status);
        // 5. 모든 작업이 끝났으므로 객체를 스스로 삭제합니다.
        delete this;
    }

private:
    TResponse Reply;
    grpc::Status Status;
    grpc::ClientContext Context;
    std::unique_ptr<grpc::ClientAsyncResponseReader<TResponse>> ResponseReader;
    CallbackFunc Callback;
};

FHktRpcProxy::FHktRpcProxy(const FString& ServerAddress)
    : bIsRunning(true)
    , bIsSyncing(false)
{
    Channel = grpc::CreateChannel(TCHAR_TO_UTF8(*ServerAddress), grpc::InsecureChannelCredentials());
    Stub = hkt::HktService::NewStub(Channel);

    CqThread = TSharedPtr<FRunnableThread>(FRunnableThread::Create(
        new FLambdaRunnable([this]() { CqLoop(); }),
        TEXT("gRPC_CompletionQueue_Thread")
    ));
}

FHktRpcProxy::~FHktRpcProxy()
{
    bIsRunning = false;
    StopSync();

    Cq.Shutdown();
    if (CqThread.IsValid())
    {
        CqThread->WaitForCompletion();
        CqThread.Reset();
    }
}

void FHktRpcProxy::StartSync(int64 InPlayerId, int32 AreaId)
{
    if (bIsSyncing) return;
    this->PlayerId = InPlayerId;
    bIsSyncing = true;
    SyncThread = TSharedPtr<FRunnableThread>(FRunnableThread::Create(
        new FLambdaRunnable([this, AreaId]() { SyncLoop(); }),
        *FString::Printf(TEXT("SyncThread_Player_%lld"), PlayerId)
    ));
}

void FHktRpcProxy::StopSync()
{
    if (!bIsSyncing) return;
    bIsSyncing = false;
    if (SyncContext)
    {
        SyncContext->TryCancel();
    }
    if (SyncThread.IsValid())
    {
        SyncThread->WaitForCompletion();
        SyncThread.Reset();
    }
}

void FHktRpcProxy::Login(const hkt::AccountRequest& Request, TFunction<void(const hkt::AccountResponse&, const grpc::Status&)> Callback)
{
    new TGenericAsyncCall<FLoginUnaryTrait>(Stub.get(), &Cq, Request, MoveTemp(Callback));
}

void FHktRpcProxy::CreateBehavior(const hkt::CreateBehaviorRequest& Request, TFunction<void(const hkt::CreateBehaviorResponse&, const grpc::Status&)> Callback)
{
    new TGenericAsyncCall<FCreateBehaviorUnaryTrait>(Stub.get(), &Cq, Request, MoveTemp(Callback));
}

void FHktRpcProxy::DestroyBehavior(const hkt::DestroyBehaviorRequest& Request, TFunction<void(const hkt::DestroyBehaviorResponse&, const grpc::Status&)> Callback)
{
    new TGenericAsyncCall<FDestroyBehaviorUnaryTrait>(Stub.get(), &Cq, Request, MoveTemp(Callback));
}

void FHktRpcProxy::SyncLoop()
{
    SyncContext = std::make_unique<grpc::ClientContext>();
    hkt::SyncRequest Request;
    Request.set_player_id(this->PlayerId);
    Request.set_area_id(0); // TODO: AreaId�� �Ķ���ͷ� �޵��� ����
    SyncReader = Stub->SyncArea(SyncContext.get(), Request);
    hkt::SyncResponse Response;
    while (bIsSyncing && SyncReader->Read(&Response))
    {
        AsyncTask(ENamedThreads::GameThread, [this, Response]() {
            OnSyncResponse.Broadcast(Response);
            });
    }
    grpc::Status Status = SyncReader->Finish();
    UE_LOG(LogTemp, Log, TEXT("Sync loop finished: %s"), UTF8_TO_TCHAR(Status.error_message().c_str()));
}

void FHktRpcProxy::CqLoop()
{
    void* Tag;
    bool bOk;
    while (Cq.Next(&Tag, &bOk))
    {
        if (!bIsRunning) break;
        if (!bOk)
        {
            UE_LOG(LogTemp, Warning, TEXT("Cq.Next() returned with bOk=false"));
            continue;
        }
        static_cast<IAsyncCall*>(Tag)->Proceed();
    }
    UE_LOG(LogTemp, Log, TEXT("Completion Queue thread finished."));
}

