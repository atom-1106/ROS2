// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ServerRpcStub.cpp
// Description: Stub implementation for control over Fast DDS RPC Server entities.

#include "ServerRpcStub.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/ReplierQos.hpp>

#include <cassert>

namespace middleware
{
namespace dds
{
ServerRpcStub::ServerRpcStub(
    uint32_t domain_id,
    std::string const& service,
    uint32_t thread_pool_size,
    ::eprosima::fastdds::dds::ReplierQos const qos,
    std::unique_ptr<::middleware::dds::IAsyncRequest>&& spImpl
)
    : m_spServerImpl{std::move(spImpl)},
      m_spFactory{::eprosima::fastdds::dds::DomainParticipantFactory::get_shared_instance()}
{
	m_pParticipant = m_spFactory->create_participant(domain_id, ::eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
	assert(m_pParticipant != nullptr);

	m_spServer = ::cat::middleware::dds::create_RequestsServer(
	    *m_pParticipant, service.c_str(), qos, thread_pool_size, m_spServerImpl
	);
	assert(m_spServer);
}
ServerRpcStub::~ServerRpcStub()
{
	m_spServer.reset();
	if(m_pParticipant != nullptr)
	{
		m_pParticipant->delete_contained_entities();
		m_spFactory->delete_participant(m_pParticipant);
	}
}
void ServerRpcStub::Run()
{
	m_spServer->run();
}
void ServerRpcStub::Stop()
{
	m_spServer->stop();
}
void ServerRpcStub::Reply(uint_least16_t key, ::middleware::Reply const& reply)
{
	m_spServerImpl->Reply(key, reply);
}
}  // namespace dds
}  // namespace middleware
