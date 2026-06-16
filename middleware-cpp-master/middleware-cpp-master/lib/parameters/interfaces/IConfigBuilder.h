// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IConfigBuilder.h
// Description: Interface for clients to build up parameter configuration.

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include "IConfigTaker.h"
#include "ParameterServiceDefines.h"

namespace middleware
{
namespace parameter
{
using OnWriteAction  = std::function<std::optional<Data>(uint_least16_t key, Identity const& id, Data const&)>;
using OnReadAction   = std::function<std::optional<Data>(uint_least16_t key, Identity const& id)>;
using OnSubscription = std::function<void(Identity const& id, Data const&)>;
using OnSubscriptionWithMetadata = std::function<void(Identity const&, Data const&, Metadata const&)>;
using OnDisconnect               = std::function<void()>;

class IConfigBuilder : public IConfigTaker
{
public:
	static std::unique_ptr<IConfigBuilder> Create(Identity&& identity);

	virtual IConfigBuilder& AddPublish(Identity&& parameter_id, Unit unit, Data&& initial_value) = 0;
	// Deprecated
	virtual IConfigBuilder&
	AddSubscribe(Identity&& parameter_id, Unit unit, OnSubscription&& callback, OnDisconnect&& disconnect) = 0;
	virtual IConfigBuilder&
	AddSubscribe(Identity&& parameter_id, OnSubscriptionWithMetadata&& callback, OnDisconnect&& disconnect) = 0;
	virtual IConfigBuilder& AddClientRead(Identity&& parameter_id, Unit unit)                               = 0;
	virtual IConfigBuilder& AddClientWrite(Identity&& parameter_id, Unit unit)                              = 0;
	virtual IConfigBuilder& AddServerWrite(Identity&& parameter_id, OnWriteAction&& callback)               = 0;
	virtual IConfigBuilder& AddServerRead(Identity&& parameter_id, OnReadAction&& callback)                 = 0;

	virtual ~IConfigBuilder() override = default;
};
}  // namespace parameter
}  // namespace middleware
