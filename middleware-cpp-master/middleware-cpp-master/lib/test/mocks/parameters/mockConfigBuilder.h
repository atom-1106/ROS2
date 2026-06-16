// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: mockConfigBuilder.h
// Description: Mock class for the ConfigBuilder interface.

#ifndef mockConfigBuilder_H
#define mockConfigBuilder_H

#include <gmock/gmock.h>
#include <middleware_parameter.pb.h>
#include "IConfigBuilder.h"

namespace mock
{
class ConfigBuilder : public ::middleware::parameter::IConfigBuilder
{
public:
	virtual ~ConfigBuilder() override = default;

	// static std::unique_ptr<IConfigBuilder> Create(::middleware::parameter::Identity&& identity) { return
	// std::make_unique<::mock::ConfigBuilder>(); }
	MOCK_METHOD(
	    ::middleware::parameter::IConfigBuilder&,
	    AddPublish,
	    (::middleware::parameter::Identity&&, ::middleware::parameter::Unit, ::middleware::parameter::Data&&)
	);
	MOCK_METHOD(
	    ::middleware::parameter::IConfigBuilder&,
	    AddSubscribe,
	    (::middleware::parameter::Identity&&,
	     ::middleware::parameter::Unit,
	     ::middleware::parameter::OnSubscription&&,
	     ::middleware::parameter::OnDisconnect&&)
	);
	MOCK_METHOD(
	    ::middleware::parameter::IConfigBuilder&,
	    AddSubscribe,
	    (::middleware::parameter::Identity&&,
	     ::middleware::parameter::OnSubscriptionWithMetadata&&,
	     ::middleware::parameter::OnDisconnect&&)
	);
	MOCK_METHOD(
	    ::middleware::parameter::IConfigBuilder&,
	    AddClientRead,
	    (::middleware::parameter::Identity&&, ::middleware::parameter::Unit)
	);
	MOCK_METHOD(
	    ::middleware::parameter::IConfigBuilder&,
	    AddClientWrite,
	    (::middleware::parameter::Identity&&, ::middleware::parameter::Unit)
	);
	MOCK_METHOD(
	    ::middleware::parameter::IConfigBuilder&,
	    AddServerWrite,
	    (::middleware::parameter::Identity&&, ::middleware::parameter::OnWriteAction&&)
	);
	MOCK_METHOD(
	    ::middleware::parameter::IConfigBuilder&,
	    AddServerRead,
	    (::middleware::parameter::Identity&&, ::middleware::parameter::OnReadAction&&)
	);
	MOCK_METHOD(::middleware::parameter::ParameterService&&, Take, ());
};
}  // namespace mock

#endif  // mockConfigBuilder_H
