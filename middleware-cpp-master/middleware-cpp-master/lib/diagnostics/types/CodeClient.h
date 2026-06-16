// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: CodeClient.h
// Description: Client for the Diagnostics Service.

#ifndef CodeClient_H
#define CodeClient_H

#include "ICodeClient.h"

namespace middleware
{
class IMiddlewareReader;
class IMiddlewareClient;
class Reply;

namespace diagnostics
{
class CodeClient final : public ::middleware::diagnostics::ICodeClient
{
public:
	CodeClient(
	    std::unique_ptr<::middleware::IMiddlewareReader>&& spSubscriptionReader,
	    std::unique_ptr<::middleware::IMiddlewareClient>&& spClient
	);
	~CodeClient() override;

	void RequestAdditionalInformation(
	    uint64_t codeKey,
	    std::vector<::middleware::diagnostics::AdditionalInformationGroup> const& groups,
	    ::middleware::diagnostics::ICodeClient::OnComplete&& additionalInformationCallback
	) override;

private:
	std::unique_ptr<::middleware::IMiddlewareReader> m_spSubscriptionReader;
	std::unique_ptr<::middleware::IMiddlewareClient> m_spClient;
};
}  // namespace diagnostics
}  // namespace middleware
#endif  // CodeClient_H
