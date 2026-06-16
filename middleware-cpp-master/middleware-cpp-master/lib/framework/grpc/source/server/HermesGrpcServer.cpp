// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: HermesGrpcServer.cpp
// Description: Implementation for gRPC Server

#include "HermesGrpcServer.h"
#include <cassert>
#include "HermesCallbackService.h"
#include "HermesGrpcTools.h"
#include "SimpleTaskSignal.h"
#include "SystemUtilities.h"

namespace middleware
{
namespace grpc
{
Server::Server(std::string const& uri, std::shared_ptr<::grpc::ServerCredentials> const& spCredentials)
    : m_serverURI(uri),
      m_spCredentials(spCredentials),
      m_spCallbackService(std::make_unique<::middleware::grpc::CallbackService>())
{
	assert(m_spCredentials);
}
Server::~Server()
{
	Stop();
}
void Server::Start()
{
	if(!m_spServerInstance)
	{
		::grpc::ServerBuilder builder;
		builder.AddListeningPort(m_serverURI, m_spCredentials);
		builder.RegisterService(m_spCallbackService.get());
		m_spServerCompletionQueue = builder.AddCompletionQueue();
		m_spServerInstance        = builder.BuildAndStart();
		::middleware::log::Info("[Server] Server listening on [%s]", m_serverURI.c_str());
	}
	else
	{
		::middleware::log::Warning("[Server] Server is already running");
	}
}
void Server::Stop()
{
	if(m_spServerInstance)
	{
		::middleware::log::Info("[Server] Grpc Server is shutting down");
		m_spServerInstance->Shutdown();
		m_spServerCompletionQueue->Shutdown();
		m_spServerInstance        = nullptr;
		m_spServerCompletionQueue = nullptr;
		::middleware::log::Info("[Server] Grpc Server has stopped");
	}
}
std::unique_ptr<::middleware::ISignalConnection> Server::RegisterOnRequest(::middleware::IServer::OnRequest&& callback)
{
	assert(callback);
	return m_spCallbackService->RegisterOnRequest(std::move(callback));
}
}  // namespace grpc
}  // namespace middleware
