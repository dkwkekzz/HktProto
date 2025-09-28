#include "HktRpcService.h"
#include <iostream>
#include <functional>

// ===================================================================
// FBehaviorSessionManager 
// ===================================================================

void FBehaviorSessionManager::BroadcastPacket(int32_t AreaId, const hkt::BehaviorPacket& Packet, int64_t ExcludePlayerId)
{
    std::lock_guard<std::mutex> lock(Mutex);
    hkt::SyncResponse response;
    *response.mutable_behavior_packet() = Packet;

    for (auto const& [playerId, stream] : Streams) {
        FSessionData data;
        if (TryGetData(playerId, data) && data.AreaId == AreaId && playerId != ExcludePlayerId) {
            stream->Write(response);
        }
    }
}

bool FBehaviorSessionManager::TryGetPlayerArea(int64_t PlayerId, int32_t& OutAreaId)
{
    FSessionData data;
    if (TryGetData(PlayerId, data)) {
        OutAreaId = data.AreaId;
        return true;
    }
    return false;
}

void FBehaviorSessionManager::RegisterCall(hkt::HktService::AsyncService* service, std::vector<std::unique_ptr<grpc::ServerCompletionQueue>>& cqs)
{
    static std::atomic<size_t> cq_index = 0;
    grpc::ServerCompletionQueue* cq = cqs[cq_index++ % cqs.size()].get();

    new TStreamingCallData<FSyncStreamingTrait>(
        service, cq,
        std::bind(&FBehaviorSessionManager::OnConnected, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&FBehaviorSessionManager::OnDisconnected, this, std::placeholders::_1)
    );
}

CallData* FBehaviorSessionManager::OnConnected(const hkt::SyncRequest& request, grpc::ServerAsyncWriter<hkt::SyncResponse>* writer)
{
    int64_t playerId = request.player_id();
    int32_t areaId = request.area_id();

    auto* stream = new TStream(playerId, writer);
    FSessionData data{ areaId };
    this->Add(playerId, data, stream);

    std::cout << "Async: Player " << playerId << " connected to Area " << areaId << std::endl;
    return stream;
}

void FBehaviorSessionManager::OnDisconnected(CallData* session_data)
{
    auto* stream = static_cast<TStream*>(session_data);
    int64_t playerId = stream->GetId();

    this->Remove(playerId);
    std::cout << "Async: Player " << playerId << " disconnected." << std::endl;
}

void FBehaviorSessionManager::Add(int64_t id, const FSessionData& data, FBehaviorSessionManager::TStream* stream) {
    std::lock_guard<std::mutex> lock(Mutex);
    Streams[id] = stream;
    SessionDatas[id] = data;
}

void FBehaviorSessionManager::Remove(int64_t id) {
    std::lock_guard<std::mutex> lock(Mutex);
    auto it = Streams.find(id);
    if (it != Streams.end()) {
        delete it->second;
        Streams.erase(it);
    }
    SessionDatas.erase(id);
}

bool FBehaviorSessionManager::TryGetData(int64_t id, FSessionData& OutData) {
    std::lock_guard<std::mutex> lock(Mutex);
    auto it = SessionDatas.find(id);
    if (it != SessionDatas.end()) {
        OutData = it->second;
        return true;
    }
    return false;
}

// ===================================================================
// FHktRpcService 
// ===================================================================

FHktRpcService::FHktRpcService()
{
}

FHktRpcService::~FHktRpcService() {
    Shutdown();
}

void FHktRpcService::Run(const std::string& server_address, size_t thread_count) {
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&Service);

    for (size_t i = 0; i < thread_count; ++i) {
        Cqs.emplace_back(builder.AddCompletionQueue());
    }

    Server = builder.BuildAndStart();
    std::cout << "Async Server listening on " << server_address << " with " << thread_count << " threads." << std::endl;

    for (size_t i = 0; i < thread_count; ++i) {
        // �� �����尡 �ڽ��� CQ�� ����ϵ��� �Ѱ���
        WorkerThreads.emplace_back(&FHktRpcService::HandleRpcs, this, i);
    }

    // --- Trait ��� RPC ������ ��� ---
    RegisterUnaryCall<FLoginUnaryTrait>(
        [this](const hkt::AccountRequest& request, hkt::AccountResponse& reply) {
            std::cout << "Async: Login request from " << request.user_id() << std::endl;
            reply.set_success(true);
            reply.set_message("Login successful!");
            reply.set_player_id(this->NextPlayerId++);
        }
    );

    RegisterUnaryCall<FCreateBehaviorUnaryTrait>(
        [this](const hkt::CreateBehaviorRequest& request, hkt::CreateBehaviorResponse& reply) {
            if (request.packets_size() > 0) {
                int64_t instigatorId = request.packets(0).instigator_id();
                int32_t areaId;
                if (this->BehaviorSessionManager.TryGetPlayerArea(instigatorId, areaId)) {
                    for (const auto& packet : request.packets()) {
                        hkt::BehaviorPacket newPacket = packet;
                        newPacket.set_behavior_id(this->NextBehaviorId++);
                        *reply.add_created_packets() = newPacket;
                        this->BehaviorSessionManager.BroadcastPacket(areaId, newPacket, instigatorId);
                    }
                }
            }
            reply.set_success(true);
        }
    );

    RegisterUnaryCall<FDestroyBehaviorUnaryTrait>(
        [this](const hkt::DestroyBehaviorRequest& request, hkt::DestroyBehaviorResponse& reply) {
            int64_t instigatorId = request.instigator_id();
            int32_t areaId;
            if (this->BehaviorSessionManager.TryGetPlayerArea(instigatorId, areaId)) {
                hkt::BehaviorPacket destroyBroadcastPacket;
                destroyBroadcastPacket.set_instigator_id(instigatorId);
                hkt::DestroyPacket* dp = new hkt::DestroyPacket();
                dp->set_behavior_id_to_destroy(request.behavior_id());
                destroyBroadcastPacket.set_allocated_destroy_packet(dp);
                this->BehaviorSessionManager.BroadcastPacket(areaId, destroyBroadcastPacket, 0);
            }
            reply.set_success(true);
        }
    );

    this->BehaviorSessionManager.RegisterCall(&Service, Cqs);
}

void FHktRpcService::Shutdown() {
    if (Server) {
        Server->Shutdown();
    }
    for (auto& cq : Cqs) {
        if (cq) {
            cq->Shutdown();
        }
    }
    for (auto& thread : WorkerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void FHktRpcService::HandleRpcs(size_t thread_index) {
    void* tag;
    bool ok;
    grpc::ServerCompletionQueue* cq = Cqs[thread_index].get();

    while (cq->Next(&tag, &ok)) {
        if (!ok) {
            std::cerr << "CQ event not ok for tag. Potential client disconnect or server shutdown." << std::endl;
        }

        if (tag) {
            // ��� �±װ� CallData �������̽��� �����Ƿ� �����ϰ� Proceed() ȣ��
            static_cast<CallData*>(tag)->Proceed();
        }
    }
}

