// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: mockMiddlewareDomainParticipantStub.h
// Description: Mock class for the DomainParticipantStub interface.

#ifndef mockMiddlewareDomainParticipantStub_H
#define mockMiddlewareDomainParticipantStub_H

#include <gmock/gmock.h>
#include "IDomainParticipantStub.h"

namespace mock
{
class DomainParticipantStub : public ::middleware::dds::IDomainParticipantStub
{
public:
	virtual ~DomainParticipantStub() override = default;
	MOCK_METHOD(std::string, Name, (), (const));
	MOCK_METHOD(uint32_t, MulticastListeningLocatorPort, ());
	MOCK_METHOD(
	    std::unique_ptr<::middleware::IMiddlewareReader>,
	    CreateReader,
	    (std::string const&,
	     ::eprosima::fastdds::dds::DataReaderQos const&,
	     ::middleware::IMiddlewareReader::SubscriptionCallback&&,
	     ::middleware::IMiddlewareReader::OnDisconnectCallback&&)
	);
	MOCK_METHOD(
	    std::unique_ptr<::middleware::IMiddlewareWriter>,
	    CreateWriter,
	    (std::string const&, ::eprosima::fastdds::dds::DataWriterQos const&)
	);
};
}  // namespace mock

#endif  // mockMiddlewareDomainParticipantStub_H
