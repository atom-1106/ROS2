// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: Subscriber.h
// Description: Generic Data Subscriber

#ifndef Subscriber_H
#define Subscriber_H

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include "messagePubSubTypes.hpp"

// discovery server
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include <chrono>
#include <climits>
#include <cstdlib>
#include <map>
#include <mutex>
#include <random>
#include <string>

//#define DISCOVERY_SERVER

class Subscriber : public ::eprosima::fastdds::dds::DataReaderListener
{
public:
	Subscriber(std::string const& name) : m_name(name), m_topicSupportType(new ::cat::middleware::MessagePubSubType())
	{
		// Create the participant
		::eprosima::fastdds::dds::DomainParticipantQos participantQos;
#ifdef DISCOVERY_SERVER
		::eprosima::fastdds::rtps::Locator_t locator;
		locator.kind = LOCATOR_KIND_UDPv4;
		::eprosima::fastdds::rtps::IPLocator::setIPv4(locator, 127, 0, 0, 1);
		locator.port = 64863;
		participantQos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(locator);
#endif

		participantQos.name("Participant_subscriber");
		if(std::string(std::getenv("COMMUNICATION_METHOD")) != "shm")
		{
			auto udp_transport               = std::make_shared<::eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
			udp_transport->maxMessageSize    = 9216;
			udp_transport->sendBufferSize    = 9216;
			udp_transport->receiveBufferSize = 9216;
			udp_transport->non_blocking_send = true;
			participantQos.transport().user_transports.push_back(udp_transport);
			participantQos.transport().use_builtin_transports = false;
		}
		m_pDomainParticipant =
		    ::eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(0, participantQos);
		m_topicSupportType.register_type(m_pDomainParticipant);
		m_pTopic = m_pDomainParticipant->create_topic(m_name, m_topicType, ::eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
		m_pSubscriber =
		    m_pDomainParticipant->create_subscriber(::eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT, nullptr);

		auto qos = ::eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT;
		qos.endpoint().history_memory_policy =
		    ::eprosima::fastdds::rtps::MemoryManagementPolicy::PREALLOCATED_MEMORY_MODE;
		qos.history().kind     = ::eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
		qos.durability().kind  = ::eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
		qos.reliability().kind = ::eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
		qos.history().depth    = 1;
		qos.resource_limits().max_samples              = 1;
		qos.resource_limits().max_instances            = 1;
		qos.resource_limits().max_samples_per_instance = 1;
		m_pReader                                      = m_pSubscriber->create_datareader(m_pTopic, qos, this);
		printf("[%s] Subscriber created.\n", m_name.c_str());
	}
	~Subscriber()
	{
		m_pSubscriber->delete_datareader(m_pReader);
		m_pDomainParticipant->delete_subscriber(m_pSubscriber);
		m_pDomainParticipant->delete_topic(m_pTopic);
		m_pDomainParticipant->delete_contained_entities();
		::eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(m_pDomainParticipant);
	}
	void on_subscription_matched(
	    ::eprosima::fastdds::dds::DataReader* pReader,
	    ::eprosima::fastdds::dds::SubscriptionMatchedStatus const& info
	) override
	{
		// Manages topic matches to publishers
		if(info.current_count_change == 1)
		{
			printf("[%s] Connected to publisher.\n", m_name.c_str());
		}
		else if(info.current_count_change == -1)
		{
			printf("[%s] Disconnected from publisher.\n", m_name.c_str());
		}
		else
		{
			printf("[%s] Match error.\n", m_name.c_str());
		}
	}
	void on_data_available(::eprosima::fastdds::dds::DataReader* pReader) override
	{
		::eprosima::fastdds::dds::SampleInfo info;
		::cat::middleware::Message message;

		if(pReader->take_next_sample(&message, &info) == ::eprosima::fastdds::dds::RETCODE_OK)
		{
			if(info.valid_data)
			{
				uint32_t value = From(message.protobuf());
				printf("[%s] received [%s]=[%u]\n", message.additional_information().c_str(), m_name.c_str(), value);
			}
			else
			{
				printf("[%s] Message data is invalid.\n", m_name.c_str());
			}
		}
	}

private:
	uint32_t From(std::vector<uint8_t> const& vector)
	{
		auto value = std::numeric_limits<uint32_t>::min();
		for(std::size_t index = 0; index < std::min(sizeof(uint32_t), vector.size()); ++index)
		{
			value |= static_cast<uint32_t>(vector.at(index)) << ((sizeof(uint32_t) - 1 - index) * CHAR_BIT);
		}
		return value;
	}

	std::string const m_name{};
	std::string const m_topicType = "cat::middleware::Message";
	::eprosima::fastdds::dds::TypeSupport m_topicSupportType;
	::eprosima::fastdds::dds::DomainParticipant* m_pDomainParticipant;
	::eprosima::fastdds::dds::Subscriber* m_pSubscriber;
	::eprosima::fastdds::dds::Topic* m_pTopic;
	::eprosima::fastdds::dds::DataReader* m_pReader;
};
#endif  // Subscriber_H
