// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IHermesCallbackService.h
// Description: Interface for the internal hermes messenger callback service

#ifndef IHermesCallbackService_H
#define IHermesCallbackService_H

#include <hermes_transaction.grpc.pb.h>
#include <functional>
#include "DefinedTypes.h"
#include "MiddlewareDefines.h"
#include "Signalling.h"

namespace middleware
{
namespace grpc
{
class ICallbackService : public ::cat::middleware::grpc::Transaction::CallbackService
{
public:
	using OnRequest = std::function<void(::middleware::Request const&, ::middleware::Reply&)>;

	virtual ~ICallbackService()                                                                      = default;
	virtual std::unique_ptr<::middleware::ISignalConnection> RegisterOnRequest(OnRequest&& callback) = 0;
};

}  // namespace grpc
}  // namespace middleware
#endif /* IHermesCallbackService_H */
