// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: mockMiddlewareRequests.h
// Description: Mock class for the IDL Requests interface.

#ifndef mockMiddlewareRequests_H
#define mockMiddlewareRequests_H

#include <gmock/gmock.h>
#include "requestsClient.hpp"

namespace mock
{
class Requests : public ::cat::middleware::dds::Requests
{
public:
	virtual ~Requests() override = default;
	MOCK_METHOD(
	    ::eprosima::fastdds::dds::rpc::RpcFuture<::cat::middleware::dds::TransferData>,
	    Read,
	    (::cat::middleware::dds::TransferData const&)
	);
	MOCK_METHOD(
	    ::eprosima::fastdds::dds::rpc::RpcFuture<::cat::middleware::dds::TransferData>,
	    Write,
	    (::cat::middleware::dds::TransferData const&)
	);
};
}  // namespace mock

#endif  // mockMiddlewareRequests_H
