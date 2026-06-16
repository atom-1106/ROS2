// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IMiddlewareParticipant.h
// Description: Factory that constructs eProsima Participants.

#ifndef IMiddlewareParticipant_H
#define IMiddlewareParticipant_H

#include "IMiddlewareReader.h"
#include "IMiddlewareWriter.h"

#include <memory>
#include <string>

namespace middleware
{
class IMiddlewareParticipant
{
public:
	virtual ~IMiddlewareParticipant()                                                                   = default;
	virtual std::unique_ptr<::middleware::IMiddlewareWriter> CreateWriter(std::string const& topicName) = 0;
	virtual std::unique_ptr<::middleware::IMiddlewareReader> CreateReader(
	    std::string const& topicName,
	    ::middleware::IMiddlewareReader::SubscriptionCallback&& on_subscription,
	    ::middleware::IMiddlewareReader::OnDisconnectCallback&& on_disconnect
	) = 0;
};
}  // namespace middleware

#endif  // IMiddlewareParticipant_H
