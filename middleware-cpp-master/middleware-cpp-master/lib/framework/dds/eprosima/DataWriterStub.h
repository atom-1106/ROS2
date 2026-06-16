// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: DataWriterStub.h
// <<Non-Testable>>
// Description: Stub for third-party library content control.

#ifndef MiddlewareDataWriterStub_H
#define MiddlewareDataWriterStub_H

#include "IDataWriterStub.h"
#include "SmartDdsPointers.h"

#include <fastdds/dds/publisher/DataWriterListener.hpp>

namespace eprosima
{
namespace fastdds
{
namespace dds
{
class DomainParticipant;
class Publisher;
class DataWriter;
class Topic;
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

namespace middleware
{
namespace dds
{
class DataWriterStub final : public ::middleware::dds::IDataWriterStub
{
public:
	static std::unique_ptr<DataWriterStub> Create(
	    ::eprosima::fastdds::dds::DomainParticipant* pDomainParticipant,
	    ::eprosima::fastdds::dds::Publisher* pPublisher,
	    ::eprosima::fastdds::dds::Topic* pTopic,            // owns
	    ::eprosima::fastdds::dds::DataWriter* pDataWriter,  // owns
	    std::unique_ptr<::eprosima::fastdds::dds::DataWriterListener>&& spDataWriterListener
	);

	DataWriterStub(
	    UniqueTopic&& spTopic,
	    UniqueDataWriter&& spDataWriter,
	    std::unique_ptr<::eprosima::fastdds::dds::DataWriterListener>&& spDataWriterListener
	);
	virtual ~DataWriterStub() override;
	::eprosima::fastdds::dds::ReturnCode_t Write(void* pMessage) override;

private:
	std::string m_topicName;
	UniqueTopic const m_spTopic;
	// m_spDataWriterListener MUST be declared before m_spDataWriter so that the DataWriter
	// is destroyed first (C++ reverse-order destruction). FastDDS calls the listener during
	// delete_datawriter; if the listener were freed first we would have a use-after-free.
	std::unique_ptr<::eprosima::fastdds::dds::DataWriterListener> const m_spDataWriterListener;
	UniqueDataWriter const m_spDataWriter;
};
}  // namespace dds
}  // namespace middleware

#endif  // MiddlewareDataWriterStub_H
