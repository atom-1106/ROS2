// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IConfigBuilder.h
// Description: Builds up parameter config for middleware.

#pragma once

#include "IConfigBuilder.h"

#include "ParameterServiceStructures.h"

namespace middleware
{
namespace parameter
{
class ConfigBuilder : public IConfigBuilder
{
public:
	ConfigBuilder(Identity&& identity);

	// IConfigBuilder
	IConfigBuilder& AddPublish(Identity&& parameter_id, Unit unit, Data&& initial_value) override;
	// Deprecated
	IConfigBuilder&
	AddSubscribe(Identity&& parameter_id, Unit unit, OnSubscription&& callback, OnDisconnect&& disconnect) override;
	IConfigBuilder&
	AddSubscribe(Identity&& parameter_id, OnSubscriptionWithMetadata&& callback, OnDisconnect&& disconnect) override;
	IConfigBuilder& AddClientRead(Identity&& parameter_id, Unit unit) override;
	IConfigBuilder& AddClientWrite(Identity&& parameter_id, Unit unit) override;
	IConfigBuilder& AddServerWrite(Identity&& parameter_id, OnWriteAction&& callback) override;
	IConfigBuilder& AddServerRead(Identity&& parameter_id, OnReadAction&& callback) override;
	ParameterService&& Take() override;

private:
	ParameterService m_config;
	bool m_hasBeenTaken{false};
};
}  // namespace parameter
}  // namespace middleware
