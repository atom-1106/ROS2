// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: HermesGrpcServer.h
// Description: Implementation for gRPC Server

#ifndef HermesGrpcServer_H
#define HermesGrpcServer_H

#include "IMiddlewareServer.h"

#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace middleware
{
namespace grpc
{
class ICallbackService;
class Server final : public ::middleware::IServer
{
public:
	Server(
	    std::string const& uri,
	    std::shared_ptr<::grpc::ServerCredentials> const& spCredentials = ::grpc::InsecureServerCredentials()
	);
	virtual ~Server() override;

	// Server interface
	void Start() override;
	void Stop() override;
	std::unique_ptr<::middleware::ISignalConnection> RegisterOnRequest(
	    ::middleware::IServer::OnRequest&& callback
	) override;

private:
	std::string m_serverURI{};
	std::shared_ptr<::grpc::ServerCredentials> m_spCredentials;
	std::unique_ptr<::middleware::grpc::ICallbackService> m_spCallbackService;
	std::shared_ptr<::grpc::Server> m_spServerInstance;
	std::shared_ptr<::grpc::ServerCompletionQueue> m_spServerCompletionQueue;
};
}  // namespace grpc
}  // namespace middleware
#endif /* HermesGrpcServer_H */
