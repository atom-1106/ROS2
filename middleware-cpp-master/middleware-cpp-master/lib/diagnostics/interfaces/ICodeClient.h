// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ICodeClient.h
// Description: Client interface for the Diagnostics Service.

#ifndef ICodeClient_H
#define ICodeClient_H

#include <middleware_diagnostics_additionalinformation.pb.h>
#include <cstdint>
#include <functional>
#include <vector>
#include "CodesStructures.h"

namespace middleware
{
namespace diagnostics
{
class ICodeClient
{
public:
	// Note: Why is the callback a reference? I would expect it to be const.
	using OnComplete =
	    std::function<void(::cat::middleware::diagnostics::AdditionalInformation& additionalInformation)>;
	virtual ~ICodeClient() = default;

	virtual void RequestAdditionalInformation(
	    uint64_t codeKey,
	    std::vector<::middleware::diagnostics::AdditionalInformationGroup> const& groups,
	    OnComplete&& additionalInformationCallback
	) = 0;
};
}  // namespace diagnostics
}  // namespace middleware
#endif  // ICodeClient_H
