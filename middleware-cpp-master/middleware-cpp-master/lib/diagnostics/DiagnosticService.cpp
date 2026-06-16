// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: DiagnosticService.cpp
// Description: Interface API for external users.

#include "DiagnosticService.h"
#include "CodeClient.h"
#include "CodeServer.h"
#include "CodesDefines.h"
#include "MiddlewareFrameworkFactory.h"
#include "SystemUtilities.h"

#include <memory>

namespace middleware
{
namespace diagnostics
{
Service::Service()
    : m_spMessageParticipant(
          ::middleware::FrameworkFactory::CreateMessageParticipant(0, ::middleware::diagnostics::connection_name)
      )
{
	assert(m_spMessageParticipant);
}
std::unique_ptr<::middleware::diagnostics::ICodeClient> Service::CreateCodeClient(CodeConsumer&& client)
{
	assert(client.callback);
	assert(client.disconnect);

	auto spClient = ::middleware::FrameworkFactory::CreateClientParticipant(
	    ::middleware::diagnostics::domainIdForReadWrite,
	    ::middleware::diagnostics::connection_name,
	    ::middleware::diagnostics::additionalinformation_connection_name,
	    ::middleware::diagnostics::threadPoolSizeForReadWrite,
	    ::middleware::diagnostics::clientRequestTimeout
	);

	auto onSubscription = [callback = std::move(client.callback)](auto const& serialized)
	{
		::middleware::log::Debug("[CodeClient] Callback called.");
		::cat::middleware::diagnostics::Codes codes{};
		codes.ParseFromArray(serialized.data(), serialized.size());
		callback(codes);
	};
	auto onDisconnect = [callback = std::move(client.disconnect)]
	{
		::middleware::log::Debug("[CodeClient] Disconnected.");
		callback();
	};
	auto spReader = m_spMessageParticipant->CreateReader(
	    ::middleware::diagnostics::connection_name, std::move(onSubscription), std::move(onDisconnect)
	);
	return std::make_unique<::middleware::diagnostics::CodeClient>(std::move(spReader), std::move(spClient));
}
std::unique_ptr<::middleware::diagnostics::ICodeServer> Service::CreateCodeServer(
    ::cat::middleware::diagnostics::Codes const& current,
    ::middleware::diagnostics::ICodeServer::OnRequest&& requestAdditionalInformation
)
{
	m_onRequest = std::move(requestAdditionalInformation);
	assert(m_onRequest);

	auto spServer = ::middleware::FrameworkFactory::CreateServerParticipant(
	    ::middleware::diagnostics::domainIdForReadWrite,
	    ::middleware::diagnostics::additionalinformation_connection_name,
	    ::middleware::diagnostics::threadPoolSizeForReadWrite,
	    [this](uint_least16_t key, auto const& request) { return RequestHandler(key, request); },
	    [this](uint_least16_t key, auto const& request) { return ::middleware::Reply{}; }
	);

	return std::make_unique<::middleware::diagnostics::CodeServer>(
	    current, m_spMessageParticipant->CreateWriter(::middleware::diagnostics::connection_name), std::move(spServer)
	);
}
std::optional<::middleware::Reply> Service::RequestHandler(uint64_t key, ::middleware::Request const& request)
{
	::middleware::Reply response{};
	::cat::middleware::diagnostics::AdditionalInformation requestProtobuf{};
	if(requestProtobuf.ParseFromArray(request.serialized_protobuf.data(), request.serialized_protobuf.size()))
	{
		auto responseProtobuf = m_onRequest(key, requestProtobuf);
		::middleware::log::Debug(
		    "[CodeServer] Read action called for additional information (code key=%d, group=%d), sending "
		    "reply.",
		    requestProtobuf.code_key(),
		    requestProtobuf.group_case()
		);
		if(responseProtobuf)
		{
			response.status.status_code  = ::middleware::StatusCode::STATUS_CODE_SUCCESS;
			response.serialized_protobuf = ::middleware::common::SerializeToVector(*responseProtobuf);
		}
		else
		{
			return std::nullopt;
		}
	}
	else
	{
		response.status.status_code    = ::middleware::StatusCode::STATUS_CODE_REQUEST_FORMAT_ERROR;
		response.status.status_message = "Request could not be parsed.";
		::middleware::log::Warning("[CodeServer] %s.", response.status.status_message.c_str());
		response.serialized_protobuf = request.serialized_protobuf;
	}
	return response;
}

}  // namespace diagnostics
}  // namespace middleware
