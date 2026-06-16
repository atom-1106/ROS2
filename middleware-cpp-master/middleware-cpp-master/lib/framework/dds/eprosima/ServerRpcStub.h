// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ServerRpcStub.h
// Description: Stub implementation for control over Fast DDS RPC Server entities.

#ifndef ServerRpcStub_H
#define ServerRpcStub_H

#include "IAsyncRequest.h"
#include "IServerRpcStub.h"
#include "requestsServer.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace eprosima
{
namespace fastdds
{
namespace dds
{
class ReplierQos;
class DomainParticipantFactory;
class DomainParticipant;
namespace rpc
{
class RpcServer;
}  // namespace rpc
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

namespace middleware
{
namespace dds
{
class ServerRpcStub : public ::middleware::dds::IServerRpcStub
{
public:
	ServerRpcStub(
	    uint32_t domain_id,
	    std::string const& service,
	    uint32_t thread_pool_size,
	    ::eprosima::fastdds::dds::ReplierQos const qos,
	    std::unique_ptr<::middleware::dds::IAsyncRequest>&& spImpl
	);
	virtual ~ServerRpcStub() override;
	void Run() override;
	void Stop() override;
	void Reply(uint_least16_t key, ::middleware::Reply const& reply) override;

private:
	std::shared_ptr<::middleware::dds::IAsyncRequest> const m_spServerImpl;
	std::shared_ptr<::eprosima::fastdds::dds::DomainParticipantFactory> m_spFactory;
	::eprosima::fastdds::dds::DomainParticipant* m_pParticipant;
	std::shared_ptr<::eprosima::fastdds::dds::rpc::RpcServer> m_spServer;
};
}  // namespace dds
}  // namespace middleware

#endif  // ServerRpcStub_H
