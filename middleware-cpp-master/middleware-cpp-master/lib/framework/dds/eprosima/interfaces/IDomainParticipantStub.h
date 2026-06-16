// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IDomainParticipantStub.h
// Description: Stub interface for third-party library content control.

#ifndef IDomainParticipantStub_H
#define IDomainParticipantStub_H

#include "IMiddlewareReader.h"
#include "IMiddlewareWriter.h"

#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

#include <memory>
#include <string>

namespace middleware
{
namespace dds
{
class IDomainParticipantStub
{
public:
	virtual ~IDomainParticipantStub()                = default;
	virtual uint32_t MulticastListeningLocatorPort() = 0;
	virtual std::unique_ptr<::middleware::IMiddlewareReader> CreateReader(
	    std::string const& topic_name,
	    ::eprosima::fastdds::dds::DataReaderQos const& qos,
	    ::middleware::IMiddlewareReader::SubscriptionCallback&& on_subscription,
	    ::middleware::IMiddlewareReader::OnDisconnectCallback&& on_disconnect
	) = 0;
	virtual std::unique_ptr<::middleware::IMiddlewareWriter>
	CreateWriter(std::string const& topic_name, ::eprosima::fastdds::dds::DataWriterQos const& qos) = 0;
};
}  // namespace dds
}  // namespace middleware

#endif /* IDomainParticipantStub_H */
