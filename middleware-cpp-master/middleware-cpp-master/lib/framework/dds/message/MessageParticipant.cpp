// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MessageParticipant.cpp
// Description: Participant that distributes Reader and Writer connections.

#include "MessageParticipant.h"
#include "DomainParticipantMessageStub.h"
#include "SystemUtilities.h"

#include <fastdds/utils/IPLocator.hpp>

#include <cassert>

namespace middleware
{
namespace dds
{
MessageParticipant::MessageParticipant(
    std::unique_ptr<::middleware::dds::IDomainParticipantStub>&& spDomainParticipantStub
)
    : m_spDomainParticipantStub(std::move(spDomainParticipantStub))
{
	assert(m_spDomainParticipantStub);
}
std::unique_ptr<::middleware::IMiddlewareWriter> MessageParticipant::CreateWriter(std::string const& topicName)
{
	auto qos                = ::eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT;
	qos.publish_mode().kind = ::eprosima::fastdds::dds::SYNCHRONOUS_PUBLISH_MODE;
	qos.endpoint().history_memory_policy =
	    ::eprosima::fastdds::rtps::MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
	qos.history().kind     = ::eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
	qos.durability().kind  = ::eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
	qos.reliability().kind = ::eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
	qos.reliability().max_blocking_time.seconds    = 0;
	qos.reliability().max_blocking_time.nanosec    = 100000000;  // 100ms
	qos.history().depth                            = 1;
	qos.resource_limits().max_samples              = 1;
	qos.resource_limits().max_instances            = 1;
	qos.resource_limits().max_samples_per_instance = 1;

	qos.reliable_writer_qos().disable_positive_acks.enabled = true;
	qos.reliable_writer_qos().disable_heartbeat_piggyback   = false;
	qos.data_sharing().off();

	return m_spDomainParticipantStub->CreateWriter(topicName, qos);
}
std::unique_ptr<::middleware::IMiddlewareReader> MessageParticipant::CreateReader(
    std::string const& topicName,
    ::middleware::IMiddlewareReader::SubscriptionCallback&& on_subscription,
    ::middleware::IMiddlewareReader::OnDisconnectCallback&& on_disconnect
)
{
	auto qos = ::eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT;
	qos.endpoint().history_memory_policy =
	    ::eprosima::fastdds::rtps::MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
	qos.history().kind     = ::eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
	qos.durability().kind  = ::eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
	qos.reliability().kind = ::eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
	qos.reliability().max_blocking_time.seconds    = 0;
	qos.reliability().max_blocking_time.nanosec    = 100000000;  // 100ms
	qos.history().depth                            = 1;
	qos.resource_limits().max_samples              = 1;
	qos.resource_limits().max_instances            = 1;
	qos.resource_limits().max_samples_per_instance = 1;

	::eprosima::fastdds::rtps::Locator_t multicast_locator;
	::eprosima::fastdds::rtps::IPLocator::setIPv4(multicast_locator, 239, 'B', 'O', 'G');
	multicast_locator.port = m_spDomainParticipantStub->MulticastListeningLocatorPort();

	qos.endpoint().multicast_locator_list.push_back(multicast_locator);
	qos.reliable_reader_qos().disable_positive_acks.enabled = true;
	qos.data_sharing().off();

	return m_spDomainParticipantStub->CreateReader(
	    topicName, qos, std::move(on_subscription), std::move(on_disconnect)
	);
}
}  // namespace dds
}  // namespace middleware
