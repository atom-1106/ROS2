// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareFrameworkFactory.cpp
// Description: Middleware Framework creation factory.

#include "MiddlewareFrameworkFactory.h"
#include "ClientRpcStub.h"
#include "DomainParticipantMessageStub.h"
#include "MessageParticipant.h"
#include "MiddlewareClient.h"
#include "MiddlewareServer.h"
#include "ServerRequestHandler.h"
#include "ServerRpcStub.h"
#include "ShmCleanup.h"
#include "SystemUtilities.h"
#include "ThreadPool.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

namespace
{
}

namespace middleware
{
std::unique_ptr<::middleware::IMiddlewareParticipant>
FrameworkFactory::CreateMessageParticipant(uint32_t domain_id, std::string const& name)
{
	ShmCleanup();
	try
	{
		auto spStub = std::make_unique<::middleware::dds::DomainParticipantMessageStub>(domain_id, name);
		return std::make_unique<::middleware::dds::MessageParticipant>(std::move(spStub));
	}
	catch(std::exception const& e)
	{
		::middleware::log::Error("[MiddlewareFrameworkFactory] [Exception] %s", e.what());
	}
	return nullptr;
}
std::unique_ptr<::middleware::IMiddlewareClient> FrameworkFactory::CreateClientParticipant(
    uint32_t domain_id,
    std::string const& name,
    std::string const& service,
    uint32_t thread_pool_size,
    std::chrono::milliseconds const timeout
)
{
	ShmCleanup();
	try
	{
		::eprosima::fastdds::dds::RequesterQos qos;
		auto spClientStub = std::make_unique<::middleware::dds::ClientRpcStub>(domain_id, service, qos);
		auto spThreadpool = std::make_unique<::middleware::ThreadPool>(thread_pool_size);
		return std::make_unique<::middleware::dds::Client>(
		    name, timeout, std::move(spClientStub), std::move(spThreadpool)
		);
	}
	catch(std::exception const& e)
	{
		::middleware::log::Error("[MiddlewareFrameworkFactory] [Exception] %s", e.what());
	}
	return nullptr;
}
std::unique_ptr<::middleware::IMiddlewareServer> FrameworkFactory::CreateServerParticipant(
    uint32_t domain_id,
    std::string const& name,
    uint32_t thread_pool_size,
    OnRequestCallback&& on_reads,
    OnRequestCallback&& on_writes
)
{
	ShmCleanup();
	try
	{
		::eprosima::fastdds::dds::ReplierQos qos;
		auto spServerHandler =
		    std::make_unique<::middleware::dds::ServerRequestHandler>(std::move(on_reads), std::move(on_writes));
		auto spServerStub = std::make_unique<::middleware::dds::ServerRpcStub>(
		    domain_id, name, thread_pool_size, qos, std::move(spServerHandler)
		);
		return std::make_unique<::middleware::dds::Server>(name, std::move(spServerStub));
	}
	catch(std::exception const& e)
	{
		::middleware::log::Error("[MiddlewareFrameworkFactory] [Exception] %s", e.what());
	}
	return nullptr;
}
}  // namespace middleware
