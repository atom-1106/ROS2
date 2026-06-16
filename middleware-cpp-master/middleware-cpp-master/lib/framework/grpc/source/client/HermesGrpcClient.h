// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: HermesGrpcClient.h
// Description: Implementation for gRPC Client

#ifndef HermesGrpcClient_H
#define HermesGrpcClient_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include "DefinedTypes.h"
#include "IMiddlewareClient.h"
#include "IStubCreator.h"

namespace middleware
{
namespace grpc
{

class Client final : public ::middleware::IClient
{
public:
	Client(std::string const& name, std::shared_ptr<::middleware::grpc::IStubCreator> const& spStubCreator);
	virtual ~Client() override;

	// IClient interface
	bool ReadRequest(::google::protobuf::Message const& message, ::middleware::IClient::OnReply&& on_complete) override;
	bool ReadRequest(std::string const& serialized, OnReply&& on_complete) override;
	bool
	WriteRequest(::google::protobuf::Message const& message, ::middleware::IClient::OnReply&& on_complete) override;
	bool WriteRequest(std::string const& serialized, OnReply&& on_complete) override;
	void Start() override;
	void Stop() override;

private:
	struct AsyncClientCall
	{
		GrpcReply reply;
		::grpc::ClientContext context;
		AsyncClientCall()
		{
			this->context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));
		}
	};
	std::string const m_service_name{};
	bool DoRequest(::middleware::grpc::GrpcRequest const& request, OnReply&& on_complete);
	void ProcessRequests();
	std::string GenerateId();
	std::shared_ptr<::middleware::grpc::IStubCreator> m_spStubCreator;
	std::shared_ptr<::cat::middleware::grpc::Transaction::Stub> m_spTransactionStub;
	std::mutex m_mutex;
	std::condition_variable m_condition;
	std::thread m_thread;
	std::atomic<bool> m_stop{false};
	std::atomic<bool> m_replyReceived{false};

	std::queue<std::unique_ptr<AsyncClientCall>> m_queue{};
};
}  // namespace grpc
}  // namespace middleware
#endif  // HermesGrpcClient_H
