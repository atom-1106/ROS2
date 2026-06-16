// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareSpy.cpp
// Description: Middleware Spy setup, methods, and Fast-DDS interfacing through DomainParticipantListener

#include "MiddlewareSpy.h"
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.hpp>
#include <fastdds/rtps/reader/ReaderDiscoveryStatus.hpp>
#include <fastdds/rtps/writer/WriterDiscoveryStatus.hpp>
#include <functional>
#include <memory>
#include <sstream>
#include "DomainParticipantMessageStub.h"
#include "MessageParticipant.h"
#include "SystemUtilities.h"
#include "messagePubSubTypes.hpp"

// TODO: only gets pub/subs, not read/writes, because only creating a listener on the one topic
//       Middleware redesign current plan is to condense to one topic, so only monitoring pub/sub for now
MiddlewareSpy::MiddlewareSpy()
    : m_topicSupportType{new ::cat::middleware::MessagePubSubType{}},
      m_spMiddlewareParticipant(
          std::make_unique<::middleware::dds::MessageParticipant>(
              std::make_unique<::middleware::dds::DomainParticipantMessageStub>(0, "MiddlewareSpy")
          )
      )
{
	::eprosima::fastdds::dds::DomainParticipantQos participantQos{};
	participantQos.name() = m_qosName;
	m_pDiscoveryParticipant =
	    ::eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(0, participantQos, this);
	m_pDiscoveryParticipant->register_type(m_topicSupportType);
	m_pDiscoverySubscriber =
	    m_pDiscoveryParticipant->create_subscriber(::eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT, nullptr);

	std::ostringstream guid_ss;
	guid_ss << m_pDiscoveryParticipant->guid().guidPrefix;

	::middleware::log::Info("[MiddlewareSpy] Discovery participants created with guid [%s].", guid_ss.str().c_str());
}

MiddlewareSpy::~MiddlewareSpy()
{
	m_pDiscoveryParticipant->delete_subscriber(m_pDiscoverySubscriber);
	m_pDiscoveryParticipant->unregister_type(m_topicSupportType.get_type_name());
	::eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(m_pDiscoveryParticipant);
}

void MiddlewareSpy::AddWiretap(std::string const& topic)
{
	::middleware::log::Info("[MiddlewareSpy] Adding wiretap for [%s].", topic.c_str());
	if(m_tappedTopics.find(topic) != m_tappedTopics.end())
	{
		m_tappedTopics.insert(topic);
	}
	else
	{
		m_tappedTopics.insert(topic);
		// will correctly call child callback despite MiddlewareSpy::topicSubscriberData being virtual
		m_middlewareSubscribers.emplace(
		    topic,
		    m_spMiddlewareParticipant->CreateReader(
		        topic, [this, name = topic](auto const& data) { topicSubscriberData(name, data); }, []() {}
		    )
		);
	}
}

void MiddlewareSpy::RemoveWiretap(std::string const& topic)
{
	m_tappedTopics.erase(topic);
	if(m_tappedTopics.find(topic) == m_tappedTopics.end())
	{
		// TODO: causes segfaults :) unsubscribing isn't really supported right now
		// m_middlewareSubscribers.erase(topic);
	}
}

void MiddlewareSpy::on_participant_discovery(
    ::eprosima::fastdds::dds::DomainParticipant*,  // this is self, useless
    ::eprosima::fastdds::rtps::ParticipantDiscoveryStatus status,
    ::eprosima::fastdds::rtps::ParticipantBuiltinTopicData const& info,
    bool& should_be_ignored
)
{

	if(!info.guid.is_on_same_process_as(m_pDiscoveryParticipant->guid()))
	{
		switch(status)
		{
			case ::eprosima::fastdds::rtps::ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT:
			{
				std::ostringstream guid_ss;
				guid_ss << m_pDiscoveryParticipant->guid().guidPrefix;
				::middleware::log::Info("[MiddlewareSpy] Discovered guid [%s].", guid_ss.str().c_str());
				m_participantData.erase(info.guid);  // as we don't clean up on remove/drop currently
				m_participantData.insert({info.guid, info});
				break;
			}
			case ::eprosima::fastdds::rtps::ParticipantDiscoveryStatus::REMOVED_PARTICIPANT:
			case ::eprosima::fastdds::rtps::ParticipantDiscoveryStatus::DROPPED_PARTICIPANT:
				// commented for now to not remove participants - they die first and we need their properties below
				// TODO: look into cleaning these up if necessary after our middleware design is solidified
				//       could be done with an index of removed participants, cleanup if in it on last pub/sub removal
				// m_participantData.erase(info.guid);
				break;
			case ::eprosima::fastdds::rtps::ParticipantDiscoveryStatus::CHANGED_QOS_PARTICIPANT:
			case ::eprosima::fastdds::rtps::ParticipantDiscoveryStatus::IGNORED_PARTICIPANT: break;
		}
	}
}

void MiddlewareSpy::on_data_reader_discovery(
    ::eprosima::fastdds::dds::DomainParticipant*,  // this is self, useless
    ::eprosima::fastdds::rtps::ReaderDiscoveryStatus status,
    ::eprosima::fastdds::rtps::SubscriptionBuiltinTopicData const& info,
    bool& should_be_ignored
)
{
	if(!info.guid.is_on_same_process_as(m_pDiscoveryParticipant->guid()))
	{
		std::string const name = m_participantData.at(info.guid).participant_name.c_str();
		::middleware::log::Info("[MiddlewareSpy] Discovered reader [%s].", name.c_str());

		std::stringstream topic{};
		topic << info.topic_name;
		MiddlewareSpyParticipantData participantData = {&info.guid, topic.str()};
		switch(status)
		{
			case ::eprosima::fastdds::rtps::ReaderDiscoveryStatus::DISCOVERED_READER:
			{
				subscriberDiscovered(participantData);
				m_subscriberData.insert({info.guid, info});
				m_topicSubscribers.insert({topic.str(), info.guid});
				break;
			}
			case ::eprosima::fastdds::rtps::ReaderDiscoveryStatus::REMOVED_READER:
			{
				subscriberRemoved(participantData);
				m_subscriberData.erase(info.guid);
				auto range = m_topicSubscribers.equal_range(topic.str());
				for(auto it = range.first; it != range.second; ++it)
				{
					if((*it).second == info.guid)
					{
						m_topicSubscribers.erase(it);
						break;
					}
				}
				break;
			}
			case ::eprosima::fastdds::rtps::ReaderDiscoveryStatus::CHANGED_QOS_READER:
			case ::eprosima::fastdds::rtps::ReaderDiscoveryStatus::IGNORED_READER: break;
		}
	}
}

void MiddlewareSpy::on_data_writer_discovery(
    ::eprosima::fastdds::dds::DomainParticipant* participant,  // this is self, useless
    ::eprosima::fastdds::rtps::WriterDiscoveryStatus status,
    ::eprosima::fastdds::rtps::PublicationBuiltinTopicData const& info,
    bool& should_be_ignored
)
{
	if(!info.guid.is_on_same_process_as(m_pDiscoveryParticipant->guid()))
	{
		std::string const name = m_participantData.at(info.guid).participant_name.c_str();

		::middleware::log::Info("[MiddlewareSpy] Discovered writer [%s].", name.c_str());
		std::stringstream topic{};
		topic << info.topic_name;
		MiddlewareSpyParticipantData participantData = {&info.guid, topic.str()};
		switch(status)
		{
			case ::eprosima::fastdds::rtps::WriterDiscoveryStatus::DISCOVERED_WRITER:

			{
				publisherDiscovered(participantData);
				if(tapTopic(topic.str()))
				{
					AddWiretap(topic.str());
				}
				m_publisherData.insert({info.guid, info});
				m_topicPublishers.insert({topic.str(), info.guid});
				break;
			}
			case ::eprosima::fastdds::rtps::WriterDiscoveryStatus::REMOVED_WRITER:
			{
				publisherRemoved(participantData);
				if(tapTopic(topic.str()))
				{
					RemoveWiretap(topic.str());
				}
				m_publisherData.erase(info.guid);
				auto range = m_topicPublishers.equal_range(topic.str());
				for(auto it = range.first; it != range.second; ++it)
				{
					if((*it).second == info.guid)
					{
						m_topicPublishers.erase(it);
						break;
					}
				}
				break;
			}
			case ::eprosima::fastdds::rtps::WriterDiscoveryStatus::CHANGED_QOS_WRITER:
			case ::eprosima::fastdds::rtps::WriterDiscoveryStatus::IGNORED_WRITER: break;
		}
	}
	else
	{
		std::ostringstream guid_ss;
		guid_ss << info.guid.guidPrefix;

		::middleware::log::Info("[MiddlewareSpy] on_data_writer_discovery ignored guid [%s].", guid_ss.str().c_str());
	}
}
