// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: CodeServer.h
// Description: Server for the Diagnostics Service.

#ifndef CodeServer_H
#define CodeServer_H

#include "ICodeServer.h"

namespace middleware
{
class IMiddlewareWriter;
class IMiddlewareServer;

namespace diagnostics
{
class CodeServer final : public ::middleware::diagnostics::ICodeServer
{
public:
	CodeServer(
	    ::cat::middleware::diagnostics::Codes const& current,
	    std::unique_ptr<::middleware::IMiddlewareWriter>&& spPublishWriter,
	    std::unique_ptr<::middleware::IMiddlewareServer>&& spServer
	);
	~CodeServer() override;
	void Reply(uint_least16_t key, ::cat::middleware::diagnostics::AdditionalInformation const& data) override;
	void Publish(::cat::middleware::diagnostics::Codes const& codes) override;

private:
	std::unique_ptr<::middleware::IMiddlewareWriter> m_spPublishWriter;
	std::unique_ptr<::middleware::IMiddlewareServer> m_spAdditionalInformationServer;
};
}  // namespace diagnostics
}  // namespace middleware
#endif  // CodeServer_H
