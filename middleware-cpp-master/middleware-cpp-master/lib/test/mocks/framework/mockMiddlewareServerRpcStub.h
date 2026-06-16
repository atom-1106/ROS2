// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: mockMiddlewareServerRpcStub.h
// Description: Mock class for the ServerRpcStub interface.

#ifndef mockMiddlewareServerRpcStub_H
#define mockMiddlewareServerRpcStub_H

#include <gmock/gmock.h>
#include "IServerRpcStub.h"

namespace mock
{
class ServerRpcStub : public ::middleware::dds::IServerRpcStub
{
public:
	virtual ~ServerRpcStub() override = default;
	MOCK_METHOD(void, Run, ());
	MOCK_METHOD(void, Stop, ());
	MOCK_METHOD(void, Reply, (uint_least16_t, ::middleware::Reply const&));
};
}  // namespace mock

#endif  // mockMiddlewareServerRpcStub_H
