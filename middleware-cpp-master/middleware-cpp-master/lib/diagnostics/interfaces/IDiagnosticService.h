// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IDiagnosticService.h
// Description: Interface API for external users.

#ifndef IDiagnosticService_H
#define IDiagnosticService_H

#include "ICodeClient.h"
#include "ICodeServer.h"

#include <optional>

namespace middleware
{
namespace diagnostics
{
class IService
{
public:
	virtual ~IService()                                                                                     = default;
	virtual std::unique_ptr<::middleware::diagnostics::ICodeClient> CreateCodeClient(CodeConsumer&& client) = 0;
	virtual std::unique_ptr<::middleware::diagnostics::ICodeServer> CreateCodeServer(
	    ::cat::middleware::diagnostics::Codes const& current,
	    ::middleware::diagnostics::ICodeServer::OnRequest&& requestAdditionalInformation
	) = 0;
};
}  // namespace diagnostics
}  // namespace middleware
#endif  // IDiagnosticService_H
