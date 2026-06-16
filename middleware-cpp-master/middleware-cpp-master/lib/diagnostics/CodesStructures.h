// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: CodesStructures.h
// Description: Structures for the Diagnostics Service.

#ifndef CodesStructures_H
#define CodesStructures_H

#include <middleware_diagnostics.pb.h>
#include <functional>

namespace middleware
{
namespace diagnostics
{
enum AdditionalInformationGroup
{
	Group2,
};

struct CodeConsumer
{
	// All callbacks are passed into entities to own. MUST BE THREAD SAFE!
	std::function<void(::cat::middleware::diagnostics::Codes const& codes)> callback;
	std::function<void()> disconnect;
};
}  // namespace diagnostics
}  // namespace middleware
#endif  // CodesStructures_H
