// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: SmartDdsPointers.h
// <<Non-Testable>>
// Description: RAII wrappers for some FastDDS classes.

#include "SmartDdsPointers.h"
#include "SystemUtilities.h"

// Fast DDS
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
// Standard
#include <cassert>

namespace middleware
{
namespace dds
{
using eprosima::fastdds::dds::RETCODE_OK;

TopicDeleter::TopicDeleter(::eprosima::fastdds::dds::DomainParticipant* pDomainParticipant)
    : m_pDomainParticipant{pDomainParticipant}
{
	assert(m_pDomainParticipant);
}
void TopicDeleter::operator()(::eprosima::fastdds::dds::Topic* pTopic)
{
	::middleware::log::Debug("[TopicDeleter] Calling delete_topic for [%s].", pTopic->get_name().c_str());
	[[maybe_unused]] auto retcode = m_pDomainParticipant->delete_topic(pTopic);
	assert(retcode == RETCODE_OK);
}
UniqueTopic::UniqueTopic(
    ::eprosima::fastdds::dds::Topic* pTopic,
    ::eprosima::fastdds::dds::DomainParticipant* pDomainParticipant
)
    : std::unique_ptr<::eprosima::fastdds::dds::Topic, TopicDeleter>{pTopic, TopicDeleter{pDomainParticipant}}
{
}

DataReaderDeleter::DataReaderDeleter(::eprosima::fastdds::dds::Subscriber* pSubscriber) : m_pSubscriber{pSubscriber}
{
	assert(m_pSubscriber);
}
void DataReaderDeleter::operator()(::eprosima::fastdds::dds::DataReader* pDataReader)
{
	::middleware::log::Debug("[DataReaderDeleter] Calling delete_datareader.");

	// If there are ever any contained entities, we will need to call
	// pDataReader->delete_contained_entities() here.

	[[maybe_unused]] auto retcode = m_pSubscriber->delete_datareader(pDataReader);
	assert(retcode == RETCODE_OK);
}
UniqueDataReader::UniqueDataReader(
    ::eprosima::fastdds::dds::DataReader* pDataReader,
    ::eprosima::fastdds::dds::Subscriber* pSubscriber
)
    : std::unique_ptr<::eprosima::fastdds::dds::DataReader, DataReaderDeleter>{
          pDataReader,
          DataReaderDeleter{pSubscriber}
      }
{
}

DataWriterDeleter::DataWriterDeleter(::eprosima::fastdds::dds::Publisher* pPublisher) : m_pPublisher{pPublisher}
{
	assert(m_pPublisher);
}
void DataWriterDeleter::operator()(::eprosima::fastdds::dds::DataWriter* pDataWriter)
{
	::middleware::log::Debug("[DataWriterDeleter] Calling delete_datawriter.");

	// If there are ever any contained entities, we will need to call
	// pDataWriter->delete_contained_entities() here.

	[[maybe_unused]] auto retcode = m_pPublisher->delete_datawriter(pDataWriter);
	assert(retcode == RETCODE_OK);
}
UniqueDataWriter::UniqueDataWriter(
    ::eprosima::fastdds::dds::DataWriter* pDataWriter,
    ::eprosima::fastdds::dds::Publisher* pPublisher
)
    : std::unique_ptr<::eprosima::fastdds::dds::DataWriter, DataWriterDeleter>{
          pDataWriter,
          DataWriterDeleter{pPublisher}
      }
{
}
}  // namespace dds
}  // namespace middleware
