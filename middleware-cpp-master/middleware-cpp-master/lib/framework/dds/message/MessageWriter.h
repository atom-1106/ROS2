// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MessageWriter.h
// Description: Generic DDS Publisher that uses a Protobuf topic.

#ifndef MiddlewareMessageWriter_H
#define MiddlewareMessageWriter_H

#include "IDataWriterStub.h"
#include "IMiddlewareWriter.h"

#include <fastdds/dds/topic/TypeSupport.hpp>
namespace eprosima
{
namespace fastdds
{
namespace dds
{
class DomainParticipant;
class Publisher;
class DataWriterQos;
class DataWriter;
class Topic;
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

// Standard
#include <memory>
#include <mutex>

namespace middleware
{
namespace dds
{
class MessageWriter final : public ::middleware::IMiddlewareWriter
{
public:
	static std::unique_ptr<MessageWriter> Create(
	    std::string const& topicName,
	    ::eprosima::fastdds::dds::DomainParticipant* pDomainParticipant,
	    ::eprosima::fastdds::dds::Publisher* pPublisher,
	    ::eprosima::fastdds::dds::TypeSupport topicSupportType,
	    ::eprosima::fastdds::dds::DataWriterQos const& qos
	);
	MessageWriter(std::string const& topicName);
	virtual ~MessageWriter() override = default;

	// Standalone Functionality
	void SetDependency(std::unique_ptr<IDataWriterStub>&& spDataWriterStub);

	// Interface
	bool ConnectedToSubscriber() override;
	bool Publish(::google::protobuf::Message const& message) override;
	bool Publish(std::vector<uint8_t> const& message) override;
	void OnPublicationMatched(bool matched) override;

private:
	std::string const m_topicName{};
	std::unique_ptr<IDataWriterStub> m_spDataWriterStub;
	std::atomic_bool m_matched{false};
};
}  // namespace dds
}  // namespace middleware

#endif  // MiddlewareMessageWriter_H
