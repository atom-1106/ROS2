// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: HermesMessageReader.cpp
// Description: Subscriber DataReader specific to Message topic.

#include "MessageReader.h"
#include "DataReaderStub.h"
#include "MiddlewareDataReaderListener.h"
#include "MiddlewareDefines.h"

// Tools
#include "SystemUtilities.h"

// Types
#include "messagePubSubTypes.hpp"

// Fast DDS
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

// Standard
#include <cassert>

namespace
{
}  // namespace

namespace middleware
{
namespace dds
{
std::unique_ptr<MessageReader> MessageReader::Create(
    std::string const& topicName,
    ::eprosima::fastdds::dds::DomainParticipant* pDomainParticipant,
    ::eprosima::fastdds::dds::Subscriber* pSubscriber,
    ::eprosima::fastdds::dds::TypeSupport topicSupportType,
    ::eprosima::fastdds::dds::DataReaderQos const& qos,
    ::middleware::IMiddlewareReader::SubscriptionCallback&& on_subscription,
    ::middleware::IMiddlewareReader::OnDisconnectCallback&& on_disconnect
)
{
	auto pTopic = pDomainParticipant->find_topic(
	    topicName, ::eprosima::fastdds::dds::Duration_t(0, ::middleware::timeouts_ns::topicSearchDuration)
	);
	if(pTopic == nullptr)
	{
		pTopic = pDomainParticipant->create_topic(
		    topicName, topicSupportType.get_type_name(), ::eprosima::fastdds::dds::TOPIC_QOS_DEFAULT
		);
	}

	assert(pTopic != nullptr);

	auto spMiddlewareDataReaderListener = std::make_unique<::middleware::dds::MiddlewareDataReaderListener>();
	auto spMessageReader =
	    std::make_unique<MessageReader>(topicName, std::move(on_subscription), std::move(on_disconnect));
	spMiddlewareDataReaderListener->SetDependency(spMessageReader.get());

	auto pDataReader = pSubscriber->create_datareader(pTopic, qos, spMiddlewareDataReaderListener.get());
	assert(pDataReader != nullptr);

	auto spDataReaderStub = ::middleware::dds::DataReaderStub::Create(
	    pDomainParticipant, pSubscriber, pTopic, pDataReader, std::move(spMiddlewareDataReaderListener)
	);

	spMessageReader->SetDependency(std::move(spDataReaderStub));

	::middleware::log::Info("[MessageReader] Created with topic name: [%s].", topicName.c_str());

	return spMessageReader;
}

MessageReader::MessageReader(
    std::string const& topicName,
    ::middleware::IMiddlewareReader::SubscriptionCallback&& on_subscription,
    ::middleware::IMiddlewareReader::OnDisconnectCallback&& on_disconnect
)
    : m_topicName{topicName}, m_onSubscription{std::move(on_subscription)}, m_onDisconnect{std::move(on_disconnect)}
{
	assert(m_onSubscription);
}
void MessageReader::SetDependency(std::unique_ptr<IDataReaderStub>&& spDataReaderStub)
{
	assert(spDataReaderStub);
	assert(!m_spDataReaderStub);

	m_spDataReaderStub = std::move(spDataReaderStub);
}

bool MessageReader::ConnectedToPublisher()
{
	return m_matched;
}
void MessageReader::OnSubscriptionMatched(bool matched)
{
	// Manages topic matches to publishers
	if(!m_matched && matched)
	{
		::middleware::log::Debug("[MessageReader] [%s] has been matched.", m_topicName.c_str());
		m_matched = true;
	}
	else if(m_matched && !matched)
	{
		::middleware::log::Debug("[MessageReader] [%s] has been unmatched.", m_topicName.c_str());
		if(m_onDisconnect)
		{
			m_onDisconnect();
		}
		m_matched = false;
	}
	else
	{
		// Do nothing
	}
}

void MessageReader::OnDataAvailable()
{
	// This is where the data from the publisher is received

	bool hasMoreSamples = true;
	while(hasMoreSamples)
	{
		::eprosima::fastdds::dds::SampleInfo info{};
		::cat::middleware::Message message{};
		auto retcode = ::eprosima::fastdds::dds::RETCODE_ERROR;
		if(m_spDataReaderStub)
		{
			retcode = m_spDataReaderStub->TakeNextSample(&message, &info);
		}
		else
		{
			::middleware::log::Warning("[MessageReader] DataReaderStub was destroyed for [%s].", m_topicName.c_str());
		}
		if(retcode == ::eprosima::fastdds::dds::RETCODE_OK)
		{
			if(info.valid_data)
			{
				m_onSubscription(message.protobuf());
			}
			else
			{
				::middleware::log::Warning("[MessageReader] Message data is invalid for [%s].", m_topicName.c_str());
			}
		}
		else
		{
			if(retcode != ::eprosima::fastdds::dds::RETCODE_NO_DATA)
			{
				::middleware::log::Warning(
				    "[MessageReader] take_next_sample failed for [%s]. Retcode: [%d]", m_topicName.c_str(), retcode
				);
			}

			hasMoreSamples = false;
		}
	}
}
}  // namespace dds
}  // namespace middleware
