#pragma once

#include "HktGrpc.h"
#include <memory>
#include <functional> // TFunction 사용을 위해

// gRPC 관련 클래스 전방 선언
namespace grpc {
    class Channel;
    class ClientContext;
    class CompletionQueue;
    class Status;
}

// 서버와의 gRPC 통신을 담당하는 클라이언트 클래스
class HKTGRPC_API FHktRpcProxy
{
public:
    FHktRpcProxy(const FString& ServerAddress);
    ~FHktRpcProxy();

    void Login(const hkt::AccountRequest& Request, TFunction<void(const hkt::AccountResponse&, const grpc::Status&)> Callback);
    void CreateBehavior(const hkt::CreateBehaviorRequest& Request, TFunction<void(const hkt::CreateBehaviorResponse&, const grpc::Status&)> Callback);
    void DestroyBehavior(const hkt::DestroyBehaviorRequest& Request, TFunction<void(const hkt::DestroyBehaviorResponse&, const grpc::Status&)> Callback);

    // 스트리밍 RPC는 라이프사이클이 다르므로 기존 인터페이스를 유지합니다.
    void StartSync(int64 PlayerId, int32 AreaId);
    void StopSync();

    // 스트림을 통해 받은 패킷을 처리할 델리게이트
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnSyncResponse, const hkt::SyncResponse&);
    FOnSyncResponse OnSyncResponse;

private:
    void SyncLoop(); // 동기화 스트림 수신 루프
    void CqLoop();   // 비동기 응답 처리 루프

    int64 PlayerId = 0;
    std::atomic<bool> bIsRunning;

    std::shared_ptr<grpc::Channel> Channel;
    std::unique_ptr<hkt::HktService::Stub> Stub;
    grpc::CompletionQueue Cq;

    // 비동기 처리를 위한 스레드
    TSharedPtr<FRunnableThread> CqThread;

    // 동기화 스트림 관련 멤버
    std::unique_ptr<grpc::ClientContext> SyncContext;
    std::unique_ptr<grpc::ClientReader<hkt::SyncResponse>> SyncReader;
    TSharedPtr<FRunnableThread> SyncThread;
    std::atomic<bool> bIsSyncing;
};
