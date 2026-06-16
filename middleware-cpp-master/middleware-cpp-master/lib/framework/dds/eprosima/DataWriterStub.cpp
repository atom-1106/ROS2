// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: DataWriterStub.cpp
// <<Non-Testable>>
// Description: Stub for third-party library content control.

#include "DataWriterStub.h"
#include "SystemUtilities.h"

// Fast DDS
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
// Standard
#include <cassert>

namespace middleware
{
namespace dds
{
std::unique_ptr<DataWriterStub> DataWriterStub::Create(
    ::eprosima::fastdds::dds::DomainParticipant* pDomainParticipant,
    ::eprosima::fastdds::dds::Publisher* pPublisher,
    ::eprosima::fastdds::dds::Topic* pTopic,            // owns
    ::eprosima::fastdds::dds::DataWriter* pDataWriter,  // owns
    std::unique_ptr<::eprosima::fastdds::dds::DataWriterListener>&& spDataWriterListener
)
{
	return std::make_unique<DataWriterStub>(
	    UniqueTopic(pTopic, pDomainParticipant),
	    UniqueDataWriter{pDataWriter, pPublisher},
	    std::move(spDataWriterListener)
	);
}
DataWriterStub::DataWriterStub(
    UniqueTopic&& spTopic,
    UniqueDataWriter&& spDataWriter,
    std::unique_ptr<::eprosima::fastdds::dds::DataWriterListener>&& spDataWriterListener
)
    : m_spTopic{std::move(spTopic)},
      m_spDataWriterListener{std::move(spDataWriterListener)},
      m_spDataWriter{std::move(spDataWriter)}

{
	assert(m_spTopic);
	assert(m_spDataWriter);
	assert(m_spDataWriterListener);
	m_topicName = m_spTopic->get_name();
	::middleware::log::Info("[DataWriterStub] Stub created for topic name: [%s].", m_topicName.c_str());
}
DataWriterStub::~DataWriterStub()
{
	::middleware::log::Debug("[DataWriterStub] [%s] destructor called.", m_topicName.c_str());
	// Detach the listener before the DataWriter is deleted (via m_spDataWriter's destructor).
	// This prevents FastDDS from calling back into the listener on a concurrent dispatch thread
	// during or after delete_datawriter.
	if(m_spDataWriter)
	{
		m_spDataWriter->set_listener(nullptr, ::eprosima::fastdds::dds::StatusMask::none());
	}
}

::eprosima::fastdds::dds::ReturnCode_t DataWriterStub::Write(void* pMessage)
{
	if(m_spDataWriter)
	{
		return m_spDataWriter->write(pMessage);
	}
	::middleware::log::Warning(
	    "[DataWriterStub] [%s] DDS DataWriter was destroyed. Can not do writes.", m_topicName.c_str()
	);
	return ::eprosima::fastdds::dds::RETCODE_ERROR;
}
}  // namespace dds
}  // namespace middleware
