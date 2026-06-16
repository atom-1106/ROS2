// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: CodeServer.cpp
// Description: Server for the Diagnostics Service.

#include "CodeServer.h"
#include "CodesDefines.h"
#include "MiddlewareFrameworkFactory.h"
#include "SystemUtilities.h"

#include <cstdint>
#include <string>
#include <vector>

using namespace std::placeholders;

namespace middleware
{
namespace diagnostics
{
CodeServer::CodeServer(
    ::cat::middleware::diagnostics::Codes const& current,
    std::unique_ptr<::middleware::IMiddlewareWriter>&& spPublishWriter,
    std::unique_ptr<::middleware::IMiddlewareServer>&& spServer
)
    : m_spPublishWriter{std::move(spPublishWriter)}, m_spAdditionalInformationServer{std::move(spServer)}
{
	assert(m_spPublishWriter);
	assert(m_spAdditionalInformationServer);

	m_spAdditionalInformationServer->Run();
	this->Publish(current);
}
CodeServer::~CodeServer()
{
	// ensure destruction / shouldStop is set before any members of this class (which may be accessed by callbacks) are destructed
	m_spPublishWriter.reset();
	m_spAdditionalInformationServer->Stop();
};
void CodeServer::Reply(uint_least16_t key, ::cat::middleware::diagnostics::AdditionalInformation const& data)
{
	::middleware::Reply reply{};
	reply.status.status_code  = ::middleware::StatusCode::STATUS_CODE_SUCCESS;
	reply.serialized_protobuf = ::middleware::common::SerializeToVector(data);
	m_spAdditionalInformationServer->Reply(key, reply);
}
void CodeServer::Publish(::cat::middleware::diagnostics::Codes const& codes)
{
	if(m_spPublishWriter->Publish(codes))
	{
		::middleware::log::Debug("[CodeServer] Published:\n%s\n.", codes.DebugString().c_str());
	}
	else
	{
		::middleware::log::Warning("[CodeServer] Publisher failed to publish:\n%s\n", codes.DebugString().c_str());
	}
}
}  // namespace diagnostics
}  // namespace middleware
