// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: PublishParameter.cpp
// Description: Interface API for external users.

#include "PublishParameter.h"
#include "MiddlewareFrameworkFactory.h"
#include "ParameterEndpointConfigurationUtilities.h"
#include "ParameterServiceStructures.h"
#include "SystemUtilities.h"

#include <middleware_parameter.pb.h>
#include <cassert>

namespace middleware
{
namespace parameter
{
PublishParameter::PublishParameter(
    ::middleware::parameter::server::PublishedParameter const& parameterConfig,
    std::shared_ptr<::middleware::IMiddlewareParticipant> const& spBroadcastParticipant
)
{
	assert(spBroadcastParticipant);
	m_lastPublished.mutable_info()->set_identity(parameterConfig.parameter_id);
	m_lastPublished.mutable_info()->set_unit(static_cast<uint32_t>(parameterConfig.unit));
	auto connection_name =
	    ::middleware::parameter::Domain::GetSubscriptionByParameterName(m_lastPublished.info().identity());

	m_spSubscriptionWriter = spBroadcastParticipant->CreateWriter(connection_name);
	assert(m_spSubscriptionWriter);
	this->Publish(parameterConfig.initial_value);
}
PublishParameter::~PublishParameter()
{
	// ensure destruction / shouldStop is set before any members of this class (which may be accessed by callbacks) are destructed
	m_spSubscriptionWriter.reset();
};
void PublishParameter::Publish(Data const& data)
{
	m_lastPublished.mutable_data()->CopyFrom(data);
	if(m_spSubscriptionWriter->Publish(m_lastPublished))
	{
		::middleware::log::Debug("[PublishParameter] Published:\n%s\n.", m_lastPublished.data().DebugString().c_str());
	}
	else
	{
		::middleware::log::Warning(
		    "[PublishParameter] Publisher failed to publish:\n%s\n.", m_lastPublished.data().DebugString().c_str()
		);
	}
}
Identity PublishParameter::Identity()
{
	return m_lastPublished.info().identity();
}
// Note: Not used but was requested for later usage
// cppcheck-suppress unusedFunction
Unit PublishParameter::Unit()
{
	return static_cast<::middleware::parameter::Unit>(m_lastPublished.info().unit());
}
}  // namespace parameter
}  // namespace middleware
