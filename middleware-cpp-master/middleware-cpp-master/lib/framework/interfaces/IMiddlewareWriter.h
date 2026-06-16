// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IMiddlewareWriter.h
// Description: Interface for Middleware's uni-directional publisher.

#ifndef IMiddlewareWriter_H
#define IMiddlewareWriter_H

#include <google/protobuf/message.h>
#include "MiddlewareDefines.h"

namespace middleware
{
class IMiddlewareWriter
{
public:
	virtual ~IMiddlewareWriter() = default;
	// Accessor
	virtual bool ConnectedToSubscriber()                             = 0;
	virtual bool Publish(::google::protobuf::Message const& message) = 0;
	virtual bool Publish(std::vector<uint8_t> const& message)        = 0;
	virtual void OnPublicationMatched(bool matched)                  = 0;
};
}  // namespace middleware
#endif  // IMiddlewareWriter_H
