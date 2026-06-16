// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ClientRpcStub.h
// Description: Stub implementation for control over Fast DDS RPC Client entities.

#ifndef ClientRpcStub_H
#define ClientRpcStub_H

#include "IClientRpcStub.h"

#include <cstdint>
#include <memory>
#include <string>

namespace eprosima
{
namespace fastdds
{
namespace dds
{
class RequesterQos;
class DomainParticipantFactory;
class DomainParticipant;
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

namespace middleware
{
namespace dds
{
class ClientRpcStub : public ::middleware::dds::IClientRpcStub
{
public:
	ClientRpcStub(uint32_t domain_id, std::string const& service, ::eprosima::fastdds::dds::RequesterQos const qos);
	virtual ~ClientRpcStub() override;
	std::weak_ptr<::cat::middleware::dds::Requests> GetInstance() override;

private:
	std::shared_ptr<::eprosima::fastdds::dds::DomainParticipantFactory> m_spFactory;
	::eprosima::fastdds::dds::DomainParticipant* m_pParticipant;
	std::shared_ptr<::cat::middleware::dds::Requests> m_spClient;
};
}  // namespace dds
}  // namespace middleware

#endif  // ClientRpcStub_H
