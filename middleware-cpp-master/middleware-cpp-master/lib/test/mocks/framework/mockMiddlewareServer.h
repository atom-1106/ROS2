// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: mockMiddlewareServer.h
// Description: Mock class for the MiddlewareServer interface.

#ifndef mockMiddlewareServer_H
#define mockMiddlewareServer_H

#include <gmock/gmock.h>
#include "IMiddlewareServer.h"

namespace mock
{
class MiddlewareServer : public ::middleware::IMiddlewareServer
{
public:
	virtual ~MiddlewareServer() override = default;
	MOCK_METHOD(void, Run, ());
	MOCK_METHOD(void, Stop, ());
	MOCK_METHOD(void, Reply, (uint_least16_t, ::middleware::Reply const&));
};
}  // namespace mock

#endif  // mockMiddlewareServer_H
