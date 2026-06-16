// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IServerRpcStub.h
// Description: Stub interface for control over Fast DDS RPC Server entities.

#ifndef IServerRpcStub_H
#define IServerRpcStub_H

#include "MiddlewareDefines.h"

#include <cstdint>
namespace middleware
{
namespace dds
{
class IServerRpcStub
{
public:
	virtual ~IServerRpcStub()                                                = default;
	virtual void Run()                                                       = 0;
	virtual void Stop()                                                      = 0;
	virtual void Reply(uint_least16_t key, ::middleware::Reply const& reply) = 0;
};
}  // namespace dds
}  // namespace middleware

#endif  // IServerRpcStub_H
