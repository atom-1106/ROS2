// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: HermesCallbackService.h
// Description: Implementation for gRPC Server

#ifndef HermesCallbackService_H
#define HermesCallbackService_H

#include "IHermesCallbackService.h"

#include <string>
#include <vector>

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

namespace middleware
{
namespace grpc
{
class CallbackService final : public ::middleware::grpc::ICallbackService
{
public:
	CallbackService();
	virtual ~CallbackService() override = default;
	std::unique_ptr<::middleware::ISignalConnection> RegisterOnRequest(
	    ::middleware::grpc::ICallbackService::OnRequest&& callback
	) override;

	::grpc::ServerUnaryReactor* RequestOnce(
	    ::grpc::CallbackServerContext* context,
	    ::cat::middleware::grpc::Request const* request,
	    ::cat::middleware::grpc::Reply* reply
	) override;

private:
	absl::Mutex m_mutex;
	::middleware::Signalling<void(::middleware::Request const&, ::middleware::Reply&)> m_onRequest{};
};

}  // namespace grpc
}  // namespace middleware
#endif /* HermesCallbackService_H */
