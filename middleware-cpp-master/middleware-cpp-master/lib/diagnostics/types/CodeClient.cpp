// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: CodeClient.cpp
// Description: Client for the Diagnostics Service.

#include "CodeClient.h"
#include "CodesDefines.h"
#include "MiddlewareDefines.h"
#include "MiddlewareFrameworkFactory.h"
#include "SystemUtilities.h"
#include "TestUtilities.h"

#include <chrono>
#include <string>

namespace middleware
{
namespace diagnostics
{
CodeClient::CodeClient(
    std::unique_ptr<::middleware::IMiddlewareReader>&& spSubscriptionReader,
    std::unique_ptr<::middleware::IMiddlewareClient>&& spClient
)
    : m_spSubscriptionReader{std::move(spSubscriptionReader)}, m_spClient{std::move(spClient)}
{
	assert(m_spSubscriptionReader);
	assert(m_spClient);
}

CodeClient::~CodeClient()
{
	m_spSubscriptionReader.reset();
	m_spClient.reset();
}

void CodeClient::RequestAdditionalInformation(
    uint64_t codeKey,
    std::vector<::middleware::diagnostics::AdditionalInformationGroup> const& groups,
    ::middleware::diagnostics::ICodeClient::OnComplete&& additionalInformationCallback
)
{
	assert(additionalInformationCallback);

	if(groups.empty())
	{
		// call immediately if groups empty, obj made because of callback signature
		::cat::middleware::diagnostics::AdditionalInformation none{};
		additionalInformationCallback(none);
	}

	for(auto&& group : groups)
	{
		::cat::middleware::diagnostics::AdditionalInformation request{};
		request.set_code_key(codeKey);
		switch(group)
		{
			case Group2: request.group_2(); break;
		}

		auto on_complete = [callback = additionalInformationCallback](auto const& reply)
		{
			::cat::middleware::diagnostics::AdditionalInformation data{};
			if(reply.status.status_code == ::middleware::StatusCode::STATUS_CODE_SUCCESS)
			{
				if(!data.ParseFromArray(reply.serialized_protobuf.data(), reply.serialized_protobuf.size()))
				{
					::middleware::log::Warning("[CodeClient] Reply could not be parsed.");
					data.Clear();
				}
			}
			else
			{
				::middleware::log::Warning(
				    "[CodeClient] Reply failed due to [%s].", reply.status.status_message.c_str()
				);
				data.Clear();
			}
			callback(data);
		};
		m_spClient->Read(request, std::move(on_complete));
	}
}
}  // namespace diagnostics
}  // namespace middleware
