// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: mockMiddlewareServerRequestHandler.h
// Description: Mock class for the IDL RequestsServer_IServerImplementation interface.

#ifndef mockMiddlewareServerRequestHandler_H
#define mockMiddlewareServerRequestHandler_H

#include <gmock/gmock.h>
#include "IAsyncRequest.h"

namespace mock
{
class ServerRequestHandler : public ::cat::middleware::dds::RequestsServer_IServerImplementation
{
public:
	virtual ~ServerRequestHandler() override = default;
	MOCK_METHOD(void, Reply, (uint_least16_t, ::middleware::Reply const&));
	MOCK_METHOD(
	    ::cat::middleware::dds::TransferData,
	    Read,
	    (::eprosima::fastdds::dds::rpc::RpcRequest const&, ::cat::middleware::dds::TransferData const&)
	);
	MOCK_METHOD(
	    ::cat::middleware::dds::TransferData,
	    Write,
	    (::eprosima::fastdds::dds::rpc::RpcRequest const&, ::cat::middleware::dds::TransferData const&)
	);
};
}  // namespace mock

#endif  // mockMiddlewareServerRequestHandler_H
