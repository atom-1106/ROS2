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
#include "message/messagePubSubTypes.hpp"

// discovery server
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include <chrono>
#include <climits>
#include <cstdlib>
#include <string>

//#define DISCOVERY_SERVER

class Subscriber : public ::eprosima::fastdds::dds::DataReaderListener
{
public:
	Subscriber(std::string const& name) : m_name(name), m_topic_support_type(new MessagePubSubType())
	{
		::eprosima::fastdds::dds::DomainParticipantQos participant_qos;
		participant_qos.name("Participant_subscriber");
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
		mp_subscriber =
		    mp_domain_participant->create_subscriber(::eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT, nullptr);

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
		mp_reader                                      = mp_subscriber->create_datareader(mp_topic, qos, this);
	}

	~Subscriber()
	{
		mp_subscriber->delete_datareader(mp_reader);
		mp_domain_participant->delete_subscriber(mp_subscriber);
		mp_domain_participant->delete_topic(mp_topic);
		mp_domain_participant->delete_contained_entities();
		::eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(mp_domain_participant);
	}

	void on_data_available(::eprosima::fastdds::dds::DataReader* pReader) override
	{
		::eprosima::fastdds::dds::SampleInfo info;
		Message message;

		if(pReader->take_next_sample(&message, &info) == ::eprosima::fastdds::dds::RETCODE_OK)
		{
			if(info.valid_data)
			{
				uint32_t value = from(message.protobuf());
				printf("[%s] received [%s]=[%u]\n", message.additional_information().c_str(), m_name.c_str(), value);
			}
		}
	}

private:
	uint32_t from(std::vector<uint8_t> const& vector)
	{
		auto value = std::numeric_limits<uint32_t>::min();
		for(std::size_t index = 0; index < std::min(sizeof(uint32_t), vector.size()); ++index)
		{
			value |= static_cast<uint32_t>(vector.at(index)) << ((sizeof(uint32_t) - 1 - index) * CHAR_BIT);
		}
		return value;
	}

	std::string const m_name{};
	::eprosima::fastdds::dds::TypeSupport m_topic_support_type;
	::eprosima::fastdds::dds::DomainParticipant* mp_domain_participant;
	::eprosima::fastdds::dds::Subscriber* mp_subscriber;
	::eprosima::fastdds::dds::Topic* mp_topic;
	::eprosima::fastdds::dds::DataReader* mp_reader;
};

#endif
