// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MessageParticipant.h
// Description: Participant that distributes Reader and Writer connections.

#ifndef MiddlewareMessageParticipant_H
#define MiddlewareMessageParticipant_H

#include "IMiddlewareParticipant.h"

#include <memory>

namespace middleware
{
namespace dds
{
class IDomainParticipantStub;
class MessageParticipant : public ::middleware::IMiddlewareParticipant
{
public:
	MessageParticipant(std::unique_ptr<::middleware::dds::IDomainParticipantStub>&& spDomainParticipantStub);
	virtual ~MessageParticipant() override = default;
	std::unique_ptr<::middleware::IMiddlewareWriter> CreateWriter(std::string const& topicName) override;
	std::unique_ptr<::middleware::IMiddlewareReader> CreateReader(
	    std::string const& topicName,
	    ::middleware::IMiddlewareReader::SubscriptionCallback&& on_subscription,
	    ::middleware::IMiddlewareReader::OnDisconnectCallback&& on_disconnect
	) override;

private:
	std::unique_ptr<::middleware::dds::IDomainParticipantStub> m_spDomainParticipantStub;
};
}  // namespace dds
}  // namespace middleware
#endif  // MiddlewareMessageParticipant_H
