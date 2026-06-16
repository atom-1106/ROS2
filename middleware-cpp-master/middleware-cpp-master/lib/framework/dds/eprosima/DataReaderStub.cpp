// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: DataReaderStub.cpp
// <<Non-Testable>>
// Description: Stub for third-party library content control.

#include "DataReaderStub.h"
#include "SystemUtilities.h"

// Fast DDS
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
// Standard
#include <cassert>

namespace middleware
{
namespace dds
{
std::unique_ptr<DataReaderStub> DataReaderStub::Create(
    ::eprosima::fastdds::dds::DomainParticipant* pDomainParticipant,
    ::eprosima::fastdds::dds::Subscriber* pSubscriber,
    ::eprosima::fastdds::dds::Topic* pTopic,            // owns
    ::eprosima::fastdds::dds::DataReader* pDataReader,  // owns
    std::unique_ptr<::eprosima::fastdds::dds::DataReaderListener>&& spDataReaderListener
)
{
	return std::make_unique<DataReaderStub>(
	    UniqueTopic(pTopic, pDomainParticipant),
	    UniqueDataReader{pDataReader, pSubscriber},
	    std::move(spDataReaderListener)
	);
}
DataReaderStub::DataReaderStub(
    UniqueTopic&& spTopic,
    UniqueDataReader&& spDataReader,
    std::unique_ptr<::eprosima::fastdds::dds::DataReaderListener>&& spDataReaderListener
)
    : m_spTopic{std::move(spTopic)},
      m_spDataReaderListener{std::move(spDataReaderListener)},
      m_spDataReader{std::move(spDataReader)}
{
	assert(m_spTopic);
	assert(m_spDataReader);
	assert(m_spDataReaderListener);

	m_topicName = m_spTopic->get_name();
	::middleware::log::Info("[DataReaderStub] Stub created for topic name: [%s].", m_topicName.c_str());
}
DataReaderStub::~DataReaderStub()
{
	::middleware::log::Debug("[DataReaderStub] [%s] destructor called.", m_topicName.c_str());
	// Detach the listener before the DataReader is deleted (via m_spDataReader's destructor).
	// This prevents FastDDS from calling back into the listener on a concurrent dispatch thread
	// during or after delete_datareader. set_listener is thread-safe and waits for any
	// in-progress listener invocation to complete before returning.
	if(m_spDataReader)
	{
		m_spDataReader->set_listener(nullptr, ::eprosima::fastdds::dds::StatusMask::none());
	}
}
::eprosima::fastdds::dds::ReturnCode_t
DataReaderStub::TakeNextSample(void* pMessage, ::eprosima::fastdds::dds::SampleInfo* pInfo)
{
	if(m_spDataReader)
	{
		return m_spDataReader->take_next_sample(pMessage, pInfo);
	}
	::middleware::log::Warning(
	    "[DataReaderStub] [%s] DDS DataReader was destroyed. Can not take samples.", m_topicName.c_str()
	);
	return ::eprosima::fastdds::dds::RETCODE_ERROR;
}
}  // namespace dds
}  // namespace middleware
