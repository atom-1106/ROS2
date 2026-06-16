// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ClientRpcStub.cpp
// Description: Stub implementation for control over Fast DDS RPC Client entities.

#include "ClientRpcStub.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/RequesterQos.hpp>

#include <cassert>

namespace middleware
{
namespace dds
{
ClientRpcStub::ClientRpcStub(
    uint32_t domain_id,
    std::string const& service,
    ::eprosima::fastdds::dds::RequesterQos const qos
)
    : m_spFactory{::eprosima::fastdds::dds::DomainParticipantFactory::get_shared_instance()}
{
	m_pParticipant = m_spFactory->create_participant(domain_id, ::eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
	assert(m_pParticipant != nullptr);

	m_spClient = ::cat::middleware::dds::create_RequestsClient(*m_pParticipant, service.c_str(), qos);
	assert(m_spClient);
}
ClientRpcStub::~ClientRpcStub()
{
	m_spClient.reset();
	if(m_pParticipant != nullptr)
	{
		m_pParticipant->delete_contained_entities();
		m_spFactory->delete_participant(m_pParticipant);
	}
}
std::weak_ptr<::cat::middleware::dds::Requests> ClientRpcStub::GetInstance()
{
	return std::weak_ptr<::cat::middleware::dds::Requests>{m_spClient};
}
}  // namespace dds
}  // namespace middleware
