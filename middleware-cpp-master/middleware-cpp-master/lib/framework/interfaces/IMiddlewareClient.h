// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IMiddlewareClient.h
// Description: Interface for Middleware's transactional client.

#ifndef IMiddlewareClient_H
#define IMiddlewareClient_H

#include <google/protobuf/message.h>
#include <functional>
#include <string>
#include "MiddlewareDefines.h"

namespace middleware
{
class IMiddlewareClient
{
public:
	virtual ~IMiddlewareClient() = default;
	virtual void Read(::google::protobuf::Message const& message, ::middleware::OnCompleteCallback&& on_complete)  = 0;
	virtual void Write(::google::protobuf::Message const& message, ::middleware::OnCompleteCallback&& on_complete) = 0;
};

}  // namespace middleware

#endif /* IMiddlewareClient_H */
