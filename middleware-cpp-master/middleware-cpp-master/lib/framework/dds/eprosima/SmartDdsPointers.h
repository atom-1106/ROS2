// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: SmartDdsPointers.h
// <<Non-Testable>>
// Description: RAII wrappers for some FastDDS classes.

#ifndef SmartDdsPointers_H
#define SmartDdsPointers_H

#include <memory>

namespace eprosima
{
namespace fastdds
{
namespace dds
{
class DataReader;
class DataWriter;
class DomainParticipant;
class Publisher;
class Subscriber;
class Topic;
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

namespace middleware
{
namespace dds
{
class TopicDeleter
{
public:
	explicit TopicDeleter(::eprosima::fastdds::dds::DomainParticipant* pDomainParticipant);
	void operator()(::eprosima::fastdds::dds::Topic* pTopic);

private:
	::eprosima::fastdds::dds::DomainParticipant* m_pDomainParticipant;
};
class UniqueTopic : public std::unique_ptr<::eprosima::fastdds::dds::Topic, TopicDeleter>
{
public:
	UniqueTopic(
	    ::eprosima::fastdds::dds::Topic* pTopic,
	    ::eprosima::fastdds::dds::DomainParticipant* pDomainParticipant
	);
};

class DataReaderDeleter
{
public:
	explicit DataReaderDeleter(::eprosima::fastdds::dds::Subscriber* pSubscriber);
	void operator()(::eprosima::fastdds::dds::DataReader* pDataReader);

private:
	::eprosima::fastdds::dds::Subscriber* m_pSubscriber;
};
class UniqueDataReader : public std::unique_ptr<::eprosima::fastdds::dds::DataReader, DataReaderDeleter>
{
public:
	UniqueDataReader(
	    ::eprosima::fastdds::dds::DataReader* pDataReader,
	    ::eprosima::fastdds::dds::Subscriber* pSubscriber
	);
};

class DataWriterDeleter
{
public:
	explicit DataWriterDeleter(::eprosima::fastdds::dds::Publisher* pPublisher);
	void operator()(::eprosima::fastdds::dds::DataWriter* pDataWriter);

private:
	::eprosima::fastdds::dds::Publisher* m_pPublisher;
};
class UniqueDataWriter : public std::unique_ptr<::eprosima::fastdds::dds::DataWriter, DataWriterDeleter>
{
public:
	UniqueDataWriter(
	    ::eprosima::fastdds::dds::DataWriter* pDataWriter,
	    ::eprosima::fastdds::dds::Publisher* pPublisher
	);
};
}  // namespace dds
}  // namespace middleware

#endif  // SmartDdsPointers_H
