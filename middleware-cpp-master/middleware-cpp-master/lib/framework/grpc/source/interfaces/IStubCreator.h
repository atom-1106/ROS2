// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IStubCreator.h
// Description: Hermes gRPC Stub factory

#ifndef IStubCreator_H
#define IStubCreator_H

#include <hermes_transaction.grpc.pb.h>
#include <functional>
#include <memory>

namespace middleware
{
namespace grpc
{
class IStubCreator
{
public:
	using OnDisconnectCallback                                                                  = std::function<void()>;
	virtual ~IStubCreator()                                                                     = default;
	virtual std::unique_ptr<::cat::middleware::grpc::Transaction::Stub> CreateTransactionStub() = 0;
	virtual void AddOnDisconnect(OnDisconnectCallback&& on_disconnect)                          = 0;
	virtual std::string Uri()                                                                   = 0;
};
}  // namespace grpc
}  // namespace middleware
#endif  // IStubCreator_H
