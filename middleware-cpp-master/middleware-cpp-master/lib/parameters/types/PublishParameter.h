// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: PublishParameter.h
// Description: Interface API for external users.

#ifndef PublishParameter_H
#define PublishParameter_H

#include <memory>
#include "IPublishParameter.h"
#include "ParameterServiceStructures.h"

namespace middleware
{
class IMiddlewareParticipant;
class IMiddlewareWriter;
}  // namespace middleware

namespace middleware
{
namespace parameter
{
class PublishParameter final : public ::middleware::parameter::IPublish
{
public:
	PublishParameter(
	    ::middleware::parameter::server::PublishedParameter const& parameterConfig,
	    std::shared_ptr<::middleware::IMiddlewareParticipant> const& spBroadcastParticipant
	);
	~PublishParameter() override;
	void Publish(Data const& data) override;
	::middleware::parameter::Identity Identity() override;
	::middleware::parameter::Unit Unit() override;

private:
	std::unique_ptr<::middleware::IMiddlewareWriter> m_spSubscriptionWriter;
	::middleware::parameter::Parameter m_lastPublished{};
};
}  // namespace parameter
}  // namespace middleware
#endif  // PublishParameter_H
