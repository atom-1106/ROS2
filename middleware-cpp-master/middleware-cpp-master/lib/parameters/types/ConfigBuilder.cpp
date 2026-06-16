// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ConfigBuilder.cpp
// Description: Interface API for external users.

#include "ConfigBuilder.h"
#include <cassert>

namespace middleware
{
namespace parameter
{
std::unique_ptr<IConfigBuilder> IConfigBuilder::Create(Identity&& identity)
{
	return std::make_unique<ConfigBuilder>(std::move(identity));
}

ConfigBuilder::ConfigBuilder(Identity&& identity) : m_config{std::move(identity)}
{
	assert(!m_config.server_identity.empty());
}
IConfigBuilder& ConfigBuilder::AddPublish(Identity&& parameter_id, Unit unit, Data&& initial_value)
{
	assert(!m_hasBeenTaken);

	m_config.published_parameters.push_back({std::move(parameter_id), unit, std::move(initial_value)});
	return *this;
}
// Deprecated
IConfigBuilder&
ConfigBuilder::AddSubscribe(Identity&& parameter_id, Unit unit, OnSubscription&& callback, OnDisconnect&& disconnect)
{
	assert(!m_hasBeenTaken);

	m_config.subscribed_parameters.push_back(
	    {std::move(parameter_id), unit, std::move(callback), nullptr, std::move(disconnect)}
	);
	return *this;
}
IConfigBuilder&
ConfigBuilder::AddSubscribe(Identity&& parameter_id, OnSubscriptionWithMetadata&& callback, OnDisconnect&& disconnect)
{
	assert(!m_hasBeenTaken);

	m_config.subscribed_parameters.push_back(
	    {std::move(parameter_id),
	     ::middleware::parameter::Unit::none,
	     nullptr,
	     std::move(callback),
	     std::move(disconnect)}
	);
	return *this;
}
IConfigBuilder& ConfigBuilder::AddClientRead(Identity&& parameter_id, Unit unit)
{
	assert(!m_hasBeenTaken);

	m_config.client_read_parameters.push_back({std::move(parameter_id), unit});
	return *this;
}
IConfigBuilder& ConfigBuilder::AddClientWrite(Identity&& parameter_id, Unit unit)
{
	assert(!m_hasBeenTaken);

	m_config.client_write_parameters.push_back({std::move(parameter_id), unit});
	return *this;
}
IConfigBuilder& ConfigBuilder::AddServerWrite(Identity&& parameter_id, OnWriteAction&& callback)
{
	assert(!m_hasBeenTaken);

	m_config.server_write_parameters.push_back({std::move(parameter_id), std::move(callback)});
	return *this;
}
IConfigBuilder& ConfigBuilder::AddServerRead(Identity&& parameter_id, OnReadAction&& callback)
{
	assert(!m_hasBeenTaken);

	m_config.server_read_parameters.push_back({std::move(parameter_id), std::move(callback)});
	return *this;
}

ParameterService&& ConfigBuilder::Take()
{
	assert(!m_hasBeenTaken);

	m_hasBeenTaken = true;
	return std::move(m_config);
}
}  // namespace parameter
}  // namespace middleware
