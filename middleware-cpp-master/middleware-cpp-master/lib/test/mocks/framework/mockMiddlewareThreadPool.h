// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: mockMiddlewareServerRpcStub.h
// Description: Mock class for the ServerRpcStub interface.

#ifndef mockMiddlewareServerRpcStub_H
#define mockMiddlewareServerRpcStub_H

#include <gmock/gmock.h>
#include "IThreadPool.h"

namespace mock
{
class ThreadPool : public ::middleware::IThreadPool
{
public:
	virtual ~ThreadPool() override = default;
	MOCK_METHOD(void, Add, (::middleware::IThreadPool::Task&&));
	MOCK_METHOD(void, Stop, ());
};
}  // namespace mock

#endif  // mockMiddlewareServerRpcStub_H
