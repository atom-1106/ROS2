// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: DomainParticipantMessageStub.h
// <<Non-Testable>>
// Description: Stub for third-party library content control.

#ifndef MiddlewareDomainParticipantMessageStub_H
#define MiddlewareDomainParticipantMessageStub_H

#include "IDomainParticipantStub.h"

#include <fastdds/dds/topic/TypeSupport.hpp>

namespace eprosima
{
namespace fastdds
{
namespace dds
{
class DomainParticipant;
class Publisher;
class Subscriber;
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

namespace middleware
{
namespace dds
{
class DomainParticipantMessageStub final : public ::middleware::dds::IDomainParticipantStub
{
public:
	DomainParticipantMessageStub(uint32_t const domain_id, std::string const& name);
	virtual ~DomainParticipantMessageStub() override;
	uint32_t MulticastListeningLocatorPort() override;
	std::unique_ptr<::middleware::IMiddlewareReader> CreateReader(
	    std::string const& topic_name,
	    ::eprosima::fastdds::dds::DataReaderQos const& qos,
	    ::middleware::IMiddlewareReader::SubscriptionCallback&& on_subscription,
	    ::middleware::IMiddlewareReader::OnDisconnectCallback&& on_disconnect
	) override;
	std::unique_ptr<::middleware::IMiddlewareWriter>
	CreateWriter(std::string const& topic_name, ::eprosima::fastdds::dds::DataWriterQos const& qos) override;

private:
	std::string const m_name{};
	::eprosima::fastdds::dds::TypeSupport m_topicSupportType;
	::eprosima::fastdds::dds::DomainParticipant* m_pDomainParticipant;
	::eprosima::fastdds::dds::Publisher* m_pPublisher;
	::eprosima::fastdds::dds::Subscriber* m_pSubscriber;
};
}  // namespace dds
}  // namespace middleware

#endif  // MiddlewareDomainParticipantMessageStub_H
