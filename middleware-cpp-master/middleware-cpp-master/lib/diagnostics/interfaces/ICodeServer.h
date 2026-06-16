// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ICodeServer.h
// Description: Server for the Diagnostics Service.

#ifndef ICodeServer_H
#define ICodeServer_H

#include <middleware_diagnostics.pb.h>
#include <middleware_diagnostics_additionalinformation.pb.h>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace middleware
{
namespace diagnostics
{
class ICodeServer
{
public:
	using Request       = ::cat::middleware::diagnostics::AdditionalInformation;
	using OptionalReply = std::optional<::cat::middleware::diagnostics::AdditionalInformation>;
	using OnRequest     = std::function<OptionalReply(uint_least16_t key, Request const&)>;

	virtual ~ICodeServer()                                                                                    = default;
	virtual void Reply(uint_least16_t key, ::cat::middleware::diagnostics::AdditionalInformation const& data) = 0;
	virtual void Publish(::cat::middleware::diagnostics::Codes const& codes)                                  = 0;
};
}  // namespace diagnostics
}  // namespace middleware
#endif  // ICodeServer_H
