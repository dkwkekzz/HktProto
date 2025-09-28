#pragma once

#include "HktGrpc.h"
#include "HktRpcTraits.h"
#include <mutex>
#include <unordered_map>
#include <atomic>
#include <deque>
#include <iostream>
#include <functional>

// ===================================================================
// CallData ���� �ӽ� ����
// ===================================================================

// �񵿱� RPC ȣ���� ���¸� �����ϴ� �⺻ Ŭ���� (�������̽�)
// ��� RPC ���� �ӽ� ��ü�� �� Ŭ������ ��ӹ޽��ϴ�.
class CallData {
public:
    virtual ~CallData() = default;
    // CompletionQueue���� �̺�Ʈ�� �߻����� �� ȣ��� �Լ�.
    // �� �Լ� ������ ���� ���¿� ���� ���� �ൿ�� �����մϴ�.
    virtual void Proceed() = 0;
};

// [���ȭ] ��� ���� ��û/���� RPC�� ���� ������ ó���ϴ� ���ø� Ŭ�����Դϴ�.
// ����(CREATE) -> ó��(PROCESS) -> �Ϸ�(FINISH) ���� �ӽ��� �ڵ����� �����մϴ�.
template <typename TUnaryCallTrait>
class TUnaryCallData final : public CallData {
public:
    using TRequest = typename TUnaryCallTrait::TRequest;
    using TReply = typename TUnaryCallTrait::TResponse;
    using ProcessLogicFunc = std::function<void(const TRequest&, TReply&)>;

    TUnaryCallData(
        hkt::HktService::AsyncService* service,
        grpc::ServerCompletionQueue* cq,
        ProcessLogicFunc process_func
    ) : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE),
        process_func_(std::move(process_func)) {
        Proceed();
    }

    void Proceed() override {
        if (status_ == CREATE) {
            status_ = PROCESS;
            (service_->*TUnaryCallTrait::RequestFunc)(&ctx_, &request_, &responder_, cq_, cq_, this);
        }
        else if (status_ == PROCESS) {
            new TUnaryCallData(service_, cq_, process_func_);
            process_func_(request_, reply_);
            status_ = FINISH;
            responder_.Finish(reply_, grpc::Status::OK, this);
        }
        else {
            delete this;
        }
    }

private:
    enum CallStatus { CREATE, PROCESS, FINISH };
    hkt::HktService::AsyncService* service_;
    grpc::ServerCompletionQueue* cq_;
    grpc::ServerContext ctx_;
    TRequest request_;
    TReply reply_;
    grpc::ServerAsyncResponseWriter<TReply> responder_;
    CallStatus status_;
    ProcessLogicFunc process_func_;
};


// [���ȭ] ��� ���� ��Ʈ���� RPC�� ���� ������ ó���ϴ� ���ø� Ŭ����.
// ����������Ŭ ����(����, ����)�� �ڵ�ȭ�մϴ�.
template <typename TStreamingCallTrait>
class TStreamingCallData final : public CallData {
public:
    using TRequest = typename TStreamingCallTrait::TRequest;
    using TReply = typename TStreamingCallTrait::TResponse;
    using OnConnectedFunc = std::function<CallData* (const TRequest&, grpc::ServerAsyncWriter<TReply>*)>;
    using OnDisconnectedFunc = std::function<void(CallData*)>;

    TStreamingCallData(
        hkt::HktService::AsyncService* service,
        grpc::ServerCompletionQueue* cq,
        OnConnectedFunc on_connected_func,
        OnDisconnectedFunc on_disconnected_func
    ) : service_(service), cq_(cq), writer_(&ctx_), status_(CREATE),
        on_connected_func_(std::move(on_connected_func)),
        on_disconnected_func_(std::move(on_disconnected_func)),
        session_data_(nullptr) {
        Proceed();
    }

    void Proceed() override {
        if (status_ == CREATE) {
            status_ = PROCESS;
            (service_->*TStreamingCallTrait::RequestFunc)(&ctx_, &request_, &writer_, cq_, cq_, this);
        }
        else if (status_ == PROCESS) {
            new TStreamingCallData(service_, cq_, on_connected_func_, on_disconnected_func_);
            session_data_ = on_connected_func_(request_, &writer_);
            status_ = FINISH;
            ctx_.AsyncNotifyWhenDone(this);
        }
        else {
            if (session_data_) {
                on_disconnected_func_(session_data_);
            }
            delete this;
        }
    }

private:
    enum CallStatus { CREATE, PROCESS, FINISH };
    hkt::HktService::AsyncService* service_;
    grpc::ServerCompletionQueue* cq_;
    grpc::ServerContext ctx_;
    TRequest request_;
    grpc::ServerAsyncWriter<TReply> writer_;
    CallStatus status_;
    OnConnectedFunc on_connected_func_;
    OnDisconnectedFunc on_disconnected_func_;
    CallData* session_data_;
};


// Ŭ (, ) ڵȭմϴ.
template <typename TResponse>
class TSessionStream : public CallData {
public:
    TSessionStream(int64_t InId, grpc::ServerAsyncWriter<TResponse>* writer)
        : Id(InId), Writer(writer), bIsWriting(false) {}

    void Proceed() override {
        std::lock_guard<std::mutex> lock(WriteMutex);
        if (!WriteQueue.empty())
        {
            WriteQueue.pop_front();
        }
        StartNextWrite();
    }

    void Write(const TResponse& response) {
        std::lock_guard<std::mutex> lock(WriteMutex);
        WriteQueue.push_back(response);
        if (!bIsWriting.exchange(true)) {
            StartNextWrite();
        }
    }

    int64_t GetId() const { return Id; }

private:
    void StartNextWrite() {
        if (WriteQueue.empty()) {
            bIsWriting = false;
            return;
        }
        const auto& response = WriteQueue.front();
        Writer->Write(response, this);
    }

    int64_t Id;
    grpc::ServerAsyncWriter<TResponse>* Writer;
    std::mutex WriteMutex;
    std::deque<TResponse> WriteQueue;
    std::atomic<bool> bIsWriting;
};


// Game-specific session manager
class FBehaviorSessionManager
{
public:
	using TStreamingTraits = FSyncStreamingTrait;
    using TStream = TSessionStream<typename FSyncStreamingTrait::TResponse>;

    void RegisterCall(
        hkt::HktService::AsyncService* service,
        std::vector<std::unique_ptr<grpc::ServerCompletionQueue>>& cqs
    );
    void BroadcastPacket(int32_t AreaId, const hkt::BehaviorPacket& Packet, int64_t ExcludePlayerId = 0);
    bool TryGetPlayerArea(int64_t PlayerId, int32_t& OutAreaId);

private:
    CallData* OnConnected(const hkt::SyncRequest& request, grpc::ServerAsyncWriter<hkt::SyncResponse>* writer);
    void OnDisconnected(CallData* session_data);

private:
    struct FSessionData
    {
        int32_t AreaId;
    };

    void Add(int64_t id, const FSessionData& data, TStream* stream);
    void Remove(int64_t id);
    bool TryGetData(int64_t id, FSessionData& OutData);

    std::unordered_map<int64_t, TStream*> Streams;
    std::unordered_map<int64_t, FSessionData> SessionDatas;
    std::mutex Mutex;
};


// FHktServiceImpl 񵿱 
//  RPC  ƴ, RPC û ް  ӽ(CallData) ϴ Ҹ մϴ.
class FHktRpcService final {
public:
    FHktRpcService();
    ~FHktRpcService();

    void Run(const std::string& server_address, size_t thread_count);
    void Shutdown();

private:
    void HandleRpcs(size_t thread_index);

    template<typename TUnaryCallTrait>
    void RegisterUnaryCall(
        typename TUnaryCallData<TUnaryCallTrait>::ProcessLogicFunc process_func)
    {
        static std::atomic<size_t> cq_index = 0;
        grpc::ServerCompletionQueue* cq = Cqs[cq_index++ % Cqs.size()].get();

        new TUnaryCallData<TUnaryCallTrait>(&Service, cq, std::move(process_func));
    }

    // gRPC  
    std::unique_ptr<grpc::Server> Server;
    hkt::HktService::AsyncService Service;
    std::vector<std::unique_ptr<grpc::ServerCompletionQueue>> Cqs;
    std::vector<std::thread> WorkerThreads;

    FBehaviorSessionManager BehaviorSessionManager;

    //     (Thread-safe)
    std::atomic<int64_t> NextPlayerId{ 1 };
    std::atomic<int64_t> NextBehaviorId{ 1 };
};

