// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MessageReader.h
// Description: Subscriber DataReader specific to Message topic.

#ifndef MiddlewareMessageReader_H
#define MiddlewareMessageReader_H

#include "IDataReaderStub.h"
#include "IMiddlewareReader.h"

#include <fastdds/dds/topic/TypeSupport.hpp>

// Standard
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace eprosima
{
namespace fastdds
{
namespace dds
{
class DomainParticipant;
class Subscriber;
class DataReaderQos;
class DataReader;
class Topic;
}  // namespace dds

}  // namespace fastdds

}  // namespace eprosima

namespace middleware
{
namespace dds
{
class MessageReader final : public ::middleware::IMiddlewareReader
{
public:
	static std::unique_ptr<MessageReader> Create(
	    std::string const& topicName,
	    ::eprosima::fastdds::dds::DomainParticipant* pDomainParticipant,
	    ::eprosima::fastdds::dds::Subscriber* pSubscriber,
	    ::eprosima::fastdds::dds::TypeSupport topicSupportType,
	    ::eprosima::fastdds::dds::DataReaderQos const& qos,
	    ::middleware::IMiddlewareReader::SubscriptionCallback&& on_subscription,
	    ::middleware::IMiddlewareReader::OnDisconnectCallback&& on_disconnect
	);
	MessageReader(
	    std::string const& topicName,
	    ::middleware::IMiddlewareReader::SubscriptionCallback&& on_subscription,
	    ::middleware::IMiddlewareReader::OnDisconnectCallback&& on_disconnect
	);
	virtual ~MessageReader() override = default;

	// Standalone Functionality
	void SetDependency(std::unique_ptr<IDataReaderStub>&& spDataReaderStub);

	// Interface
	bool ConnectedToPublisher() override;
	void OnSubscriptionMatched(bool matched) override;
	void OnDataAvailable() override;

private:
	std::string const m_topicName{};
	::middleware::IMiddlewareReader::SubscriptionCallback const m_onSubscription;
	::middleware::IMiddlewareReader::OnDisconnectCallback const m_onDisconnect;
	std::unique_ptr<IDataReaderStub> m_spDataReaderStub;
	std::atomic_bool m_matched{false};
};
}  // namespace dds
}  // namespace middleware

#endif  // MiddlewareMessageReader_H
