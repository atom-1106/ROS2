// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IMiddlewareServer.h
// Description: Interface for Middleware's transactional server.

#ifndef IMiddlewareServer_H
#define IMiddlewareServer_H

#include "MiddlewareDefines.h"

#include <cstdint>
namespace middleware
{
class IMiddlewareServer
{
public:
	virtual ~IMiddlewareServer()                                             = default;
	virtual void Run()                                                       = 0;
	virtual void Stop()                                                      = 0;
	virtual void Reply(uint_least16_t key, ::middleware::Reply const& reply) = 0;
};
}  // namespace middleware
#endif  // IMiddlewareServer_H
