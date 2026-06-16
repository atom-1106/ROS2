// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: HermesGrpcClient.cpp
// Description: Implementation for gRPC Client

#include "HermesGrpcClient.h"
#include "HermesGrpcTools.h"
#include "SystemUtilities.h"

#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <stdexcept>

namespace
{
}  // namespace

namespace middleware
{
namespace grpc
{
Client::Client(std::string const& name, std::shared_ptr<::middleware::grpc::IStubCreator> const& spStubCreator)
    : m_service_name(name), m_spStubCreator(spStubCreator)
{
	assert(!m_service_name.empty());
	assert(m_spStubCreator);
}
Client::~Client()
{
	Stop();
}

void Client::Start()
{
	{
		std::lock_guard<std::mutex> lock{m_mutex};
		m_stop.store(false);
	}
	::middleware::log::Info("[Client] grpc Client initialized");
	m_thread = std::thread([this] { ProcessRequests(); });
}

void Client::Stop()
{
	{
		std::lock_guard<std::mutex> lock{m_mutex};
		m_stop.store(true);
		m_condition.notify_all();
	}
	if(m_thread.joinable())
	{
		m_thread.join();
	}
}

bool Client::ReadRequest(::google::protobuf::Message const& message, ::middleware::IClient::OnReply&& on_complete)
{
	return ReadRequest(message.SerializeAsString(), std::move(on_complete));
}
bool Client::ReadRequest(std::string const& serialized, ::middleware::IClient::OnReply&& on_complete)
{
	::middleware::grpc::GrpcRequest request{};
	request.set_client_id(GenerateId());
	request.set_type(::cat::middleware::grpc::Request::TYPE_READ);
	request.set_serialized_protobuf(serialized);
	return DoRequest(request, std::move(on_complete));
}
bool Client::WriteRequest(::google::protobuf::Message const& message, ::middleware::IClient::OnReply&& on_complete)
{
	return WriteRequest(message.SerializeAsString(), std::move(on_complete));
}
bool Client::WriteRequest(std::string const& serialized, ::middleware::IClient::OnReply&& on_complete)
{
	::middleware::grpc::GrpcRequest request{};
	request.set_client_id(GenerateId());
	request.set_type(::cat::middleware::grpc::Request::TYPE_WRITE);
	request.set_serialized_protobuf(serialized);
	return DoRequest(request, std::move(on_complete));
}

bool Client::DoRequest(::middleware::grpc::GrpcRequest const& request, OnReply&& on_complete)
{
	if(!m_spTransactionStub)
	{
		m_spTransactionStub = m_spStubCreator->CreateTransactionStub();
	}
	if(m_spTransactionStub)
	{
		{
			std::lock_guard<std::mutex> lock{m_mutex};
			m_queue.push(std::make_unique<AsyncClientCall>());
		}

		auto handler = [callback = std::move(on_complete), id = request.client_id(), this](auto status)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			::middleware::Reply reply{};
			if(status.ok())
			{
				reply = ::middleware::grpc::tools::FromReply(m_queue.front()->reply);
			}
			else
			{
				reply           = ::middleware::grpc::tools::ReplyNotOk(status);
				reply.client_id = id;
			}
			callback(reply);
			m_replyReceived.store(true);
			m_condition.notify_one();
		};
		::middleware::log::Info("[Client] grpc Request with ID [%s] was sent", request.client_id().c_str());
		m_spTransactionStub->async()->RequestOnce(
		    &m_queue.back()->context, &request, &m_queue.back()->reply, std::move(handler)
		);
		return true;
	}

	return false;
}

void Client::ProcessRequests()
{
	// Block until the next result is available in the completion queue "cq".
	while(true)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_condition.wait(lock, [this] { return (m_replyReceived || m_stop); });

		if(!m_stop)
		{
			::middleware::log::Info("[Client] grpc Request ID [%s]", << m_queue.front()->reply.client_id().c_str());
			m_queue.pop();
			m_replyReceived.store(false);
		}
		else
		{
			break;
		}
	}
}
std::string Client::GenerateId()
{
	auto now         = std::chrono::system_clock::now();
	auto since_epoch = now.time_since_epoch();
	auto millis      = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch);
	return (m_service_name + "_" + std::to_string(millis.count()));
}
}  // namespace grpc
}  // namespace middleware
