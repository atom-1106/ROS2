// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: DataReaderStub.h
// <<Non-Testable>>
// Description: Stub for third-party library content control.

#ifndef MiddlewareDataReaderStub_H
#define MiddlewareDataReaderStub_H

#include "IDataReaderStub.h"
#include "SmartDdsPointers.h"

#include <fastdds/dds/subscriber/DataReaderListener.hpp>

namespace eprosima
{
namespace fastdds
{
namespace dds
{
class DomainParticipant;
class Subscriber;
class DataReader;
class Topic;
}  // namespace dds

}  // namespace fastdds

}  // namespace eprosima

namespace middleware
{
namespace dds
{
class DataReaderStub final : public ::middleware::dds::IDataReaderStub
{
public:
	static std::unique_ptr<DataReaderStub> Create(
	    ::eprosima::fastdds::dds::DomainParticipant* pDomainParticipant,
	    ::eprosima::fastdds::dds::Subscriber* pSubscriber,
	    ::eprosima::fastdds::dds::Topic* pTopic,            // owns
	    ::eprosima::fastdds::dds::DataReader* pDataReader,  // owns
	    std::unique_ptr<::eprosima::fastdds::dds::DataReaderListener>&& spDataReaderListener
	);

	DataReaderStub(
	    UniqueTopic&& spTopic,
	    UniqueDataReader&& spDataReader,
	    std::unique_ptr<::eprosima::fastdds::dds::DataReaderListener>&& spDataReaderListener
	);
	virtual ~DataReaderStub() override;
	::eprosima::fastdds::dds::ReturnCode_t
	TakeNextSample(void* pMessage, ::eprosima::fastdds::dds::SampleInfo* pInfo) override;

private:
	std::string m_topicName;
	UniqueTopic const m_spTopic;
	// m_spDataReaderListener MUST be declared before m_spDataReader so that the DataReader
	// is destroyed first (C++ reverse-order destruction). FastDDS calls the listener during
	// delete_datareader; if the listener were freed first we would have a use-after-free.
	std::unique_ptr<::eprosima::fastdds::dds::DataReaderListener> const m_spDataReaderListener;
	UniqueDataReader const m_spDataReader;
};
}  // namespace dds
}  // namespace middleware

#endif  // MiddlewareDataReaderStub_H
