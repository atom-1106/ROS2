// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ParameterServiceStructures.h
// Description: {Blitz} Defined Structures for the middleware.

#ifndef ParameterServiceStructures_H
#define ParameterServiceStructures_H

#include <middleware_parameter.pb.h>
#include <any>
#include <functional>
#include <string>
#include <vector>
#include "ParameterServiceDefines.h"

namespace middleware
{
namespace parameter
{
// All callbacks are passed into entities to own. MUST BE THREAD SAFE!
using OnWriteAction =
    std::function<std::optional<Data>(uint_least16_t key, ::middleware::parameter::Identity const&, Data const&)>;
using OnReadAction   = std::function<std::optional<Data>(uint_least16_t key, ::middleware::parameter::Identity const&)>;
using OnSubscription = std::function<void(::middleware::parameter::Identity const&, Data const&)>;
using OnSubscriptionWithMetadata =
    std::function<void(::middleware::parameter::Identity const&, Data const&, Metadata const&)>;
using OnDisconnect = std::function<void()>;

namespace client
{
struct SubscribedParameter
{
	::middleware::parameter::Identity const parameter_id;
	::middleware::parameter::Unit const unit;
	OnSubscription const callback;
	OnSubscriptionWithMetadata const callbackWithMetadata;
	OnDisconnect const disconnect;
};
struct WriteParameter
{
	::middleware::parameter::Identity const parameter_id;
	::middleware::parameter::Unit const unit;
};
struct ReadParameter
{
	::middleware::parameter::Identity const parameter_id;
	::middleware::parameter::Unit const unit;
};
}  // namespace client

namespace server
{
struct PublishedParameter
{
	::middleware::parameter::Identity const parameter_id;
	::middleware::parameter::Unit const unit;
	Data const initial_value;
};
struct WriteParameter
{
	// Server actions for a parameter written to it
	::middleware::parameter::Identity const parameter_id;
	OnWriteAction const callback;
};
struct ReadParameter
{
	// Server actions for a parameter is requested
	::middleware::parameter::Identity const parameter_id;
	OnReadAction const callback;
};
}  // namespace server

struct ParameterService
{
	::middleware::parameter::Identity server_identity{};
	std::vector<server::PublishedParameter> published_parameters{};
	std::vector<server::WriteParameter> server_write_parameters{};
	std::vector<server::ReadParameter> server_read_parameters{};

	std::vector<client::SubscribedParameter> subscribed_parameters{};
	std::vector<client::WriteParameter> client_write_parameters{};
	std::vector<client::ReadParameter> client_read_parameters{};
};

}  // namespace parameter
}  // namespace middleware
#endif  // ParameterServiceStructures_H
