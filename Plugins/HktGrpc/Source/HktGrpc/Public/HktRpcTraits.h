#pragma once

#include "HktGrpc.h"

namespace grpc {
    class Status;
}

// --- RPC Trait 정의 ---
// 실제 프로젝트에서는 이 Trait들을 별도의 헤더 파일로 분리할 수 있습니다.
struct FLoginUnaryTrait {
    using TRequest = hkt::AccountRequest;
    using TResponse = hkt::AccountResponse;
    static constexpr auto PrepareFunc = &hkt::HktService::Stub::PrepareAsyncLogin;
    static constexpr auto RequestFunc = &hkt::HktService::AsyncService::RequestLogin;
};

struct FCreateBehaviorUnaryTrait {
    using TRequest = hkt::CreateBehaviorRequest;
    using TResponse = hkt::CreateBehaviorResponse;
    static constexpr auto PrepareFunc = &hkt::HktService::Stub::PrepareAsyncCreateBehavior;
    static constexpr auto RequestFunc = &hkt::HktService::AsyncService::RequestCreateBehavior;
};

struct FDestroyBehaviorUnaryTrait {
    using TRequest = hkt::DestroyBehaviorRequest;
    using TResponse = hkt::DestroyBehaviorResponse;
    static constexpr auto PrepareFunc = &hkt::HktService::Stub::PrepareAsyncDestroyBehavior;
    static constexpr auto RequestFunc = &hkt::HktService::AsyncService::RequestDestroyBehavior;
};

struct FSyncStreamingTrait {
    using TRequest = hkt::SyncRequest;
    using TResponse = hkt::SyncResponse;
	using TPacket = hkt::BehaviorPacket;
    static constexpr auto PrepareFunc = &hkt::HktService::Stub::PrepareAsyncSyncArea;
    static constexpr auto RequestFunc = &hkt::HktService::AsyncService::RequestSyncArea;
};
