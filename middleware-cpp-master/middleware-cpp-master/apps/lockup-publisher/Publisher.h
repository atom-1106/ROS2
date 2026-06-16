// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: Publisher.h
// Description: Generic Data Publisher

#ifndef Publisher_H
#define Publisher_H

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include "messagePubSubTypes.hpp"

// discovery server
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <string>

//#define DISCOVERY_SERVER

class Publisher : public ::eprosima::fastdds::dds::DataWriterListener
{
public:
	Publisher(std::string const& name, uint32_t const min, uint32_t const max)
	    : m_name(name),
	      m_minValue(min),
	      m_maxValue(max),
	      m_currentValue(m_minValue),
	      m_topicSupportType(new ::cat::middleware::MessagePubSubType())
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

		participantQos.name("Participant_publisher");
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
		m_pPublisher = m_pDomainParticipant->create_publisher(::eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT, nullptr);

		auto qos = ::eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT;
		qos.endpoint().history_memory_policy =
		    ::eprosima::fastdds::rtps::MemoryManagementPolicy::PREALLOCATED_MEMORY_MODE;
		qos.history().kind     = ::eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
		qos.durability().kind  = ::eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
		qos.reliability().kind = ::eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
		qos.history().depth    = 1;
		qos.resource_limits().max_samples              = 1;
		qos.resource_limits().max_instances            = 1;
		qos.resource_limits().max_samples_per_instance = 1;
		m_pDataWriter                                  = m_pPublisher->create_datawriter(m_pTopic, qos, this);
		printf("[%s] Publisher created.\n", m_name.c_str());
	}
	~Publisher()
	{
		m_pPublisher->delete_datawriter(m_pDataWriter);
		m_pDomainParticipant->delete_publisher(m_pPublisher);
		m_pDomainParticipant->delete_topic(m_pTopic);
		m_pDomainParticipant->delete_contained_entities();
		::eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(m_pDomainParticipant);
	}
	void Publish()
	{
		::cat::middleware::Message data;
		data.additional_information(GetSystemTime());
		data.protobuf(To(m_currentValue));
		m_pDataWriter->write(&data);

		// Set next value
		if(m_currentValue == m_maxValue)
		{
			m_currentValue = m_minValue;
		}
		else
		{
			++m_currentValue;
		}
	}
	void on_publication_matched(
	    ::eprosima::fastdds::dds::DataWriter* spWriter,
	    ::eprosima::fastdds::dds::PublicationMatchedStatus const& info
	) override
	{
		if(info.current_count_change == 1)
		{
			printf("[%s] Connected to subscriber.\n", m_name.c_str());
		}
		else if(info.current_count_change == -1)
		{
			printf("[%s] Disconnected from subscriber.\n", m_name.c_str());
		}
		else
		{
			printf("[%s] Match error.\n", m_name.c_str());
		}
	}
	void on_offered_deadline_missed(
	    ::eprosima::fastdds::dds::DataWriter* writer,
	    ::eprosima::fastdds::dds::OfferedDeadlineMissedStatus const& status
	) override
	{
		printf("[%s] on_offered_deadline_missed not implemented\n", m_name.c_str());
	}
	void on_offered_incompatible_qos(
	    ::eprosima::fastdds::dds::DataWriter* writer,
	    ::eprosima::fastdds::dds::OfferedIncompatibleQosStatus const& status
	) override
	{
		printf("[%s] on_offered_incompatible_qos not implemented\n", m_name.c_str());
	}
	void on_liveliness_lost(
	    ::eprosima::fastdds::dds::DataWriter* writer,
	    ::eprosima::fastdds::dds::LivelinessLostStatus const& status
	) override
	{
		printf("[%s] on_liveliness_lost not implemented\n", m_name.c_str());
	}
	void on_unacknowledged_sample_removed(
	    ::eprosima::fastdds::dds::DataWriter* writer,
	    ::eprosima::fastdds::dds::InstanceHandle_t const& instance
	) override
	{
		printf("[%s] on_unacknowledged_sample_removed not implemented\n", m_name.c_str());
	}

private:
	std::vector<uint8_t> To(uint32_t const raw)
	{
		std::vector<uint8_t> data;
		for(std::size_t bitshift = (sizeof(uint32_t) - 1) * 8; bitshift > 0; bitshift -= 8)
		{
			data.push_back(static_cast<uint8_t>(raw >> bitshift));
		}
		data.push_back(static_cast<uint8_t>(raw));
		return data;
	}
	std::string GetSystemTime()
	{
		static constexpr char FORMAT[] = "%Y-%m-%d %X";
		using CLOCK                    = std::chrono::system_clock;
		auto in_time_t                 = CLOCK::to_time_t(CLOCK::now());
		std::stringstream ss;
		struct tm buf;
		gmtime_r(&in_time_t, &buf);
		ss << std::put_time(&buf, FORMAT);
		return ss.str();
	}

	std::string const m_name{};
	uint32_t const m_minValue{0};
	uint32_t const m_maxValue{0};
	uint32_t m_currentValue = 0;

	std::string const m_topicType = "cat::middleware::Message";
	::eprosima::fastdds::dds::TypeSupport m_topicSupportType;

	::eprosima::fastdds::dds::DomainParticipant* m_pDomainParticipant;
	::eprosima::fastdds::dds::Publisher* m_pPublisher;
	::eprosima::fastdds::dds::Topic* m_pTopic;
	::eprosima::fastdds::dds::DataWriter* m_pDataWriter;
};
#endif  // Publisher_H
