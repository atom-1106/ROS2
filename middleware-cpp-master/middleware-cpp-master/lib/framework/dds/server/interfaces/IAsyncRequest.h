// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IAsyncRequest.h
// Description: Handles Async Server replies for requests.

#ifndef IAsyncRequest_H
#define IAsyncRequest_H

#include "MiddlewareDefines.h"
#include "requestsServer.hpp"

#include <memory>

namespace middleware
{
namespace dds
{
class IAsyncRequest : public ::cat::middleware::dds::RequestsServer_IServerImplementation
{
public:
	virtual ~IAsyncRequest()                                                        = default;
	virtual void Reply(uint_least16_t requestKey, ::middleware::Reply const& reply) = 0;
};
}  // namespace dds
}  // namespace middleware

#endif  // IAsyncRequest_H
