// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IClientRpcStub.h
// Description: Stub interface for control over Fast DDS RPC Client entities.

#ifndef IClientRpcStub_H
#define IClientRpcStub_H

#include "requestsClient.hpp"

#include <memory>

namespace middleware
{
namespace dds
{
class IClientRpcStub
{
public:
	virtual ~IClientRpcStub()                                             = default;
	virtual std::weak_ptr<::cat::middleware::dds::Requests> GetInstance() = 0;
};
}  // namespace dds
}  // namespace middleware

#endif  // IClientRpcStub_H
