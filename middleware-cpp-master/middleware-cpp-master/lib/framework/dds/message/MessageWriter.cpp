// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MessageWriter.cpp
// Description: Generic DDS Publisher that uses a Protobuf topic.

#include "MessageWriter.h"
#include "DataWriterStub.h"
#include "MiddlewareDataWriterListener.h"
#include "MiddlewareDefines.h"

// Tools
#include "SystemUtilities.h"

// Types
#include "messagePubSubTypes.hpp"

// Fast DDS
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>

// Standard
#include <cassert>

namespace middleware
{
namespace dds
{
std::unique_ptr<MessageWriter> MessageWriter::Create(
    std::string const& topicName,
    ::eprosima::fastdds::dds::DomainParticipant* pDomainParticipant,
    ::eprosima::fastdds::dds::Publisher* pPublisher,
    ::eprosima::fastdds::dds::TypeSupport topicSupportType,
    ::eprosima::fastdds::dds::DataWriterQos const& qos
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

	auto spMiddlewareDataWriterListener = std::make_unique<::middleware::dds::MiddlewareDataWriterListener>();
	auto spMessageWriter                = std::make_unique<MessageWriter>(topicName);
	spMiddlewareDataWriterListener->SetDependency(spMessageWriter.get());

	auto pDataWriter = pPublisher->create_datawriter(pTopic, qos, spMiddlewareDataWriterListener.get());
	assert(pDataWriter != nullptr);

	auto spDataWriterStub = ::middleware::dds::DataWriterStub::Create(
	    pDomainParticipant, pPublisher, pTopic, pDataWriter, std::move(spMiddlewareDataWriterListener)
	);
	spMessageWriter->SetDependency(std::move(spDataWriterStub));

	::middleware::log::Info("[MessageWriter] Created with topic name: [%s].", topicName.c_str());

	return spMessageWriter;
}
MessageWriter::MessageWriter(std::string const& topicName) : m_topicName{topicName}
{
}
void MessageWriter::SetDependency(std::unique_ptr<IDataWriterStub>&& spDataWriterStub)
{
	assert(spDataWriterStub);
	assert(!m_spDataWriterStub);
	m_spDataWriterStub = std::move(spDataWriterStub);
}
bool MessageWriter::ConnectedToSubscriber()
{
	return m_matched;
}
bool MessageWriter::Publish(::google::protobuf::Message const& message)
{
	auto serialized = message.SerializeAsString();
	std::vector<uint8_t> bytes(std::cbegin(serialized), std::cend(serialized));
	return Publish(bytes);
}
bool MessageWriter::Publish(std::vector<uint8_t> const& message)
{
	// This is where the data is published based on the number of matched subscribers
	if(message.size() <= ::middleware::max::n_MaxPayloadByteCount)
	{
		::cat::middleware::Message data;
		data.protobuf(message);

		auto return_code = ::eprosima::fastdds::dds::RETCODE_ERROR;
		if(m_spDataWriterStub)
		{
			return_code = m_spDataWriterStub->Write(&data);
		}
		else
		{
			::middleware::log::Warning("[MessageReader] DataReaderStub was destroyed for [%s].", m_topicName.c_str());
		}

		::middleware::log::Debug("[MessageWriter] Write returned code %u.", return_code);
		return (return_code == ::eprosima::fastdds::dds::RETCODE_OK);
	}
	else
	{
		::middleware::log::Warning(
		    "[MessageWriter] Content exceeded %u bytes.", ::middleware::max::n_MaxPayloadByteCount
		);
	}
	return false;
}
void MessageWriter::OnPublicationMatched(bool matched)
{
	if(!m_matched && matched)
	{
		::middleware::log::Debug("[MessageWriter] [%s] has been matched.", m_topicName.c_str());
		m_matched = true;
	}
	else if(m_matched && !matched)
	{
		::middleware::log::Debug("[MessageWriter] [%s] has been unmatched.", m_topicName.c_str());
		m_matched = false;
	}
	else
	{
		// Do nothing
	}
}
}  // namespace dds
}  // namespace middleware
