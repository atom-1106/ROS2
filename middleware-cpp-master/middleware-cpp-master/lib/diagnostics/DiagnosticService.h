// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: DiagnosticService.h
// Description: Interface API for external users.

#ifndef DiagnosticService_H
#define DiagnosticService_H

#include "IDiagnosticService.h"

#include <optional>

namespace middleware
{
class Request;
class Reply;
class IMiddlewareParticipant;

namespace diagnostics
{
class Service final : public ::middleware::diagnostics::IService
{
public:
	Service();
	virtual ~Service() override = default;

	// Any number of receivers can be created.
	std::unique_ptr<::middleware::diagnostics::ICodeClient> CreateCodeClient(CodeConsumer&& client) override;

	// All callbacks are passed into entities to own. MUST BE THREAD SAFE!
	std::unique_ptr<::middleware::diagnostics::ICodeServer> CreateCodeServer(
	    ::cat::middleware::diagnostics::Codes const& current,
	    ::middleware::diagnostics::ICodeServer::OnRequest&& requestAdditionalInformation
	) override;

private:
	std::optional<::middleware::Reply> RequestHandler(uint64_t key, ::middleware::Request const& request);

	std::shared_ptr<::middleware::IMiddlewareParticipant> m_spMessageParticipant;
	::middleware::diagnostics::ICodeServer::OnRequest m_onRequest;
};
}  // namespace diagnostics
}  // namespace middleware
#endif  // DiagnosticService_H
