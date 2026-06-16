// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: mockMiddlewareClientRpcStub.h
// Description: Mock class for the ClientRpcStub interface.

#ifndef mockMiddlewareClientRpcStub_H
#define mockMiddlewareClientRpcStub_H

#include <gmock/gmock.h>
#include "IClientRpcStub.h"

namespace mock
{
class ClientRpcStub : public ::middleware::dds::IClientRpcStub
{
public:
	virtual ~ClientRpcStub() override = default;
	MOCK_METHOD(std::weak_ptr<::cat::middleware::dds::Requests>, GetInstance, ());
};
}  // namespace mock

#endif  // mockMiddlewareClientRpcStub_H
