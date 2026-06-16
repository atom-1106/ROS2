// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: mockEprosimaRpcRequest.h
// Description: Mock class for the Eprosima RpcRequest interface.

#ifndef mockEprosimaRpcRequest_H
#define mockEprosimaRpcRequest_H

#include <gmock/gmock.h>
#include "requestsServer.hpp"

namespace mock
{
class RpcRequest : public ::eprosima::fastdds::dds::rpc::RpcRequest
{
public:
	virtual ~RpcRequest() override = default;
	MOCK_METHOD(::eprosima::fastdds::rtps::GUID_t const&, get_client_id, (), (const));
	MOCK_METHOD(::eprosima::fastdds::rtps::RemoteLocatorList const&, get_client_locators, (), (const));
};
}  // namespace mock

#endif  // mockEprosimaRpcRequest_H
