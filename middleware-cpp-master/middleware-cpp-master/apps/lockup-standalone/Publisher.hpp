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
#include <cstdlib>
#include <iomanip>
#include <string>

//#define DISCOVERY_SERVER

class Publisher : public ::eprosima::fastdds::dds::DataWriterListener
{
public:
	Publisher(std::string const& name) : m_name(name), m_topic_support_type(new MessagePubSubType())
	{
		::eprosima::fastdds::dds::DomainParticipantQos participant_qos;
		participant_qos.name("Participant_publisher");
#ifdef DISCOVERY_SERVER
		::eprosima::fastdds::rtps::Locator_t locator;
		locator.kind = LOCATOR_KIND_UDPv4;
		::eprosima::fastdds::rtps::IPLocator::setIPv4(locator, 127, 0, 0, 1);
		locator.port = 64863;
		participantQos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(locator);
#endif
		if(std::getenv("COMMUNICATION_METHOD") != NULL && std::string(std::getenv("COMMUNICATION_METHOD")) != "shm")
		{
			auto udp_transport               = std::make_shared<::eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
			udp_transport->maxMessageSize    = 9216;
			udp_transport->sendBufferSize    = 9216;
			udp_transport->receiveBufferSize = 9216;
			udp_transport->non_blocking_send = true;
			participant_qos.transport().user_transports.push_back(udp_transport);
			participant_qos.transport().use_builtin_transports = false;
		}
		mp_domain_participant =
		    ::eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(0, participant_qos);
		m_topic_support_type.register_type(mp_domain_participant, "Message");
		mp_topic = mp_domain_participant->create_topic(m_name, "Message", ::eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
		mp_publisher =
		    mp_domain_participant->create_publisher(::eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT, nullptr);

		auto qos = ::eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT;
		qos.endpoint().history_memory_policy =
		    ::eprosima::fastdds::rtps::MemoryManagementPolicy::PREALLOCATED_MEMORY_MODE;
		qos.history().kind     = ::eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
		qos.durability().kind  = ::eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
		qos.reliability().kind = ::eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
		qos.reliability().max_blocking_time.seconds    = 0;
		qos.reliability().max_blocking_time.nanosec    = 100000000;  // 100ms
		qos.history().depth                            = 1;
		qos.resource_limits().max_samples              = 1;
		qos.resource_limits().max_instances            = 1;
		qos.resource_limits().max_samples_per_instance = 1;
		mp_data_writer                                 = mp_publisher->create_datawriter(mp_topic, qos, this);
	}

	~Publisher()
	{
		mp_publisher->delete_datawriter(mp_data_writer);
		mp_domain_participant->delete_publisher(mp_publisher);
		mp_domain_participant->delete_topic(mp_topic);
		mp_domain_participant->delete_contained_entities();
		::eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(mp_domain_participant);
	}

	void publish()
	{
		Message data;
		data.additional_information(system_time());
		data.protobuf(to(m_current_value = (m_current_value + 1) % 100));
		mp_data_writer->write(&data);
	}

private:
	std::vector<uint8_t> to(uint32_t const raw)
	{
		std::vector<uint8_t> data;
		for(std::size_t bitshift = (sizeof(uint32_t) - 1) * 8; bitshift > 0; bitshift -= 8)
		{
			data.push_back(static_cast<uint8_t>(raw >> bitshift));
		}
		data.push_back(static_cast<uint8_t>(raw));
		return data;
	}

	std::string system_time()
	{
		auto in_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		struct tm buf;
		gmtime_r(&in_time_t, &buf);
		std::stringstream ss;
		ss << std::put_time(&buf, "%Y-%m-%d %X");
		return ss.str();
	}

	std::string const m_name{};
	uint32_t m_current_value = 0;
	::eprosima::fastdds::dds::TypeSupport m_topic_support_type;
	::eprosima::fastdds::dds::DomainParticipant* mp_domain_participant;
	::eprosima::fastdds::dds::Publisher* mp_publisher;
	::eprosima::fastdds::dds::Topic* mp_topic;
	::eprosima::fastdds::dds::DataWriter* mp_data_writer;
};

#endif
