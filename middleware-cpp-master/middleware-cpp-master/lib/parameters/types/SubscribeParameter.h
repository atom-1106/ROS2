// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: SubscribeParameter.h
// Description: Interface API for external users.

#ifndef SubscribeParameter_H
#define SubscribeParameter_H

#include <middleware_parameter.pb.h>
#include "IBasicParameter.h"
#include "ParameterServiceDefines.h"
#include "ParameterServiceStructures.h"

#include <memory>
#include <mutex>

namespace middleware
{
class IMiddlewareParticipant;
class IMiddlewareReader;
}  // namespace middleware

namespace middleware
{
namespace parameter
{
class SubscribeParameter final : public ::middleware::parameter::IBasic
{
public:
	SubscribeParameter(
	    ::middleware::parameter::client::SubscribedParameter const& parameterConfig,
	    std::shared_ptr<::middleware::IMiddlewareParticipant> const& spBroadcastParticipant
	);
	~SubscribeParameter() override;
	::middleware::parameter::Identity Identity() override;
	::middleware::parameter::Unit Unit() override;

private:
	void ProtectedUserSubscriptionCall();
	void ProtectedUserDisconnectCall();

	std::mutex m_mutex;
	Parameter m_lastReceived{};
	::middleware::parameter::OnSubscription m_callback{nullptr};
	::middleware::parameter::OnSubscriptionWithMetadata m_callbackWithMetadata{nullptr};
	::middleware::parameter::OnDisconnect m_disconnect{nullptr};
	// m_spSubscriptionReader MUST be declared last (destroyed first). The reader's destructor
	// calls set_listener(nullptr) then delete_datareader; if the disconnect callback fires
	// during that teardown it dereferences m_mutex/m_lastReceived/m_callback — all of which
	// must still be alive. Declaring the reader last guarantees this (C++ reverse-order destruction).
	std::unique_ptr<::middleware::IMiddlewareReader> m_spSubscriptionReader;
};
}  // namespace parameter
}  // namespace middleware
#endif  // SubscribeParameter_H
