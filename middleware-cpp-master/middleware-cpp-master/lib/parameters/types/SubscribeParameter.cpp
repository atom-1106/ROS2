// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: SubscribeParameter.cpp
// Description: Interface API for external users.

#include "SubscribeParameter.h"
#include "MiddlewareFrameworkFactory.h"
#include "ParameterEndpointConfigurationUtilities.h"
#include "SystemUtilities.h"

#include <cassert>

namespace middleware
{
namespace parameter
{
SubscribeParameter::SubscribeParameter(
    ::middleware::parameter::client::SubscribedParameter const& parameterConfig,
    std::shared_ptr<::middleware::IMiddlewareParticipant> const& spBroadcastParticipant
)
    : m_callback{parameterConfig.callback},
      m_callbackWithMetadata{parameterConfig.callbackWithMetadata},
      m_disconnect{parameterConfig.disconnect}
{
	assert(spBroadcastParticipant);
	m_lastReceived.mutable_info()->set_identity(parameterConfig.parameter_id);
	auto connection_name =
	    ::middleware::parameter::Domain::GetSubscriptionByParameterName(m_lastReceived.info().identity());

	auto handler = [this](auto const& serialized)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		::middleware::log::Debug("[SubscribeParameter] %s callback called.", m_lastReceived.info().identity().c_str());
		::middleware::parameter::Parameter parameter{};
		m_lastReceived.ParseFromString(std::string(std::cbegin(serialized), std::cend(serialized)));
		ProtectedUserSubscriptionCall();
	};
	auto onDisconnect = [this]
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		::middleware::log::Debug("[SubscribeParameter] %s disconnected.", m_lastReceived.info().identity().c_str());
		m_lastReceived.mutable_data()->set_quality(::cat::middleware::parameter::QUALITY_BAD);
		ProtectedUserSubscriptionCall();
		ProtectedUserDisconnectCall();
	};
	m_spSubscriptionReader =
	    spBroadcastParticipant->CreateReader(connection_name, std::move(handler), std::move(onDisconnect));
	assert(m_spSubscriptionReader);
}
SubscribeParameter::~SubscribeParameter() = default;
::middleware::parameter::Identity SubscribeParameter::Identity()
{
	return m_lastReceived.info().identity();
}
::middleware::parameter::Unit SubscribeParameter::Unit()
{
	return static_cast<::middleware::parameter::Unit>(m_lastReceived.info().unit());
}
void SubscribeParameter::ProtectedUserSubscriptionCall()
{
	try
	{
		if(m_callback)
		{
			m_callback(m_lastReceived.info().identity(), m_lastReceived.data());
		}
		if(m_callbackWithMetadata)
		{
			::middleware::parameter::Metadata metadata{
			    static_cast<::middleware::parameter::Unit>(m_lastReceived.info().unit())
			};
			m_callbackWithMetadata(m_lastReceived.info().identity(), m_lastReceived.data(), metadata);
		}
	}
	catch(std::exception const& e)
	{
		::middleware::log::Warning(
		    "[SubscribeParameter] %s User Subscription Callback Exception: %s",
		    m_lastReceived.info().identity().c_str(),
		    e.what()
		);
	}
}
void SubscribeParameter::ProtectedUserDisconnectCall()
{
	try
	{
		m_disconnect();
	}
	catch(std::exception const& e)
	{
		::middleware::log::Warning(
		    "[SubscribeParameter] %s User Disconnect Callback Exception: %s",
		    m_lastReceived.info().identity().c_str(),
		    e.what()
		);
	}
}
}  // namespace parameter
}  // namespace middleware
