// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: mockMiddlewareClient.h
// Description: Mock class for the MiddlewareClient interface.

#ifndef mockMiddlewareClient_H
#define mockMiddlewareClient_H

#include <gmock/gmock.h>
#include "IMiddlewareClient.h"

namespace mock
{
class MiddlewareClient : public ::middleware::IMiddlewareClient
{
public:
	virtual ~MiddlewareClient() override = default;
	MOCK_METHOD(void, Read, (::google::protobuf::Message const&, ::middleware::OnCompleteCallback&&));
	MOCK_METHOD(void, Write, (::google::protobuf::Message const&, ::middleware::OnCompleteCallback&&));
};
}  // namespace mock

#endif  // mockMiddlewareClient_H
