// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: HermesCallbackService.cpp
// Description: Implementation for gRPC Server

#include "HermesCallbackService.h"
#include <cassert>
#include <sstream>
#include "HermesGrpcTools.h"
#include "SimpleTaskSignal.h"
#include "SystemUtilities.h"

namespace middleware
{
namespace grpc
{
CallbackService::CallbackService()
{
}

std::unique_ptr<::middleware::ISignalConnection> CallbackService::RegisterOnRequest(
    ::middleware::grpc::ICallbackService::OnRequest&& callback
)
{
	assert(callback);
	return m_onRequest.Register(std::move(callback));
}
::grpc::ServerUnaryReactor* CallbackService::RequestOnce(
    ::grpc::CallbackServerContext* context,
    ::cat::middleware::grpc::Request const* request,
    ::cat::middleware::grpc::Reply* reply
)
{
	::grpc::Status status{::grpc::StatusCode::OK, ""};
	{
		::middleware::Request hermes_request = ::middleware::grpc::tools::FromRequest(*request);
		::middleware::Reply hermes_reply;
		::middleware::SimpleTaskSignal task([&hermes_request, &hermes_reply, this]
		                                    { m_onRequest.FireEvent(hermes_request, hermes_reply); });
		if(!task.Wait())
		{
			std::stringstream msg{};
			msg << "Timeout occurred for ID [" << request->client_id() << "]";
			::middleware::log::Warning("[CallbackService] %s", msg.str().c_str());
			reply->CopyFrom(::middleware::grpc::tools::ReplyTimeout(hermes_request));
			status = ::grpc::Status{::grpc::StatusCode::DEADLINE_EXCEEDED, msg.str()};
		}
		else
		{
			reply->CopyFrom(::middleware::grpc::tools::ToReply(hermes_reply));
		}
	}

	auto* reactor = context->DefaultReactor();
	reactor->Finish(status);
	return reactor;
}
}  // namespace grpc
}  // namespace middleware
