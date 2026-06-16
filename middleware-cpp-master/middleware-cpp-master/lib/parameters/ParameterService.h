// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ParameterService.h
// Description: Interface API for external users.

#ifndef ParameterService_H
#define ParameterService_H

#include "IParameterService.h"
#include "ParameterServiceDefines.h"

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <vector>

namespace middleware
{
class IMiddlewareClient;
class IMiddlewareServer;
class IMiddlewareParticipant;
class Request;
class Reply;

namespace parameter
{
class IBasic;
class IPublish;
class IRead;
class IWrite;
class ParameterService;

class Service final : public ::middleware::parameter::IService
{
public:
	Service(::middleware::parameter::ParameterService const& configuration);
	virtual ~Service() override;
	::middleware::parameter::IPublish& RetrievePublisherInstance(
	    ::middleware::parameter::Identity const& identity
	) override;
	::middleware::parameter::IRead& RetrieveReadInstance(::middleware::parameter::Identity const& identity) override;
	::middleware::parameter::IWrite& RetrieveWriteInstance(::middleware::parameter::Identity const& identity) override;
	void Reply(uint_least16_t key, Data const& data) override;

private:
	using OnWriteAction = std::function<
	    std::optional<Data>(uint_least16_t key, ::middleware::parameter::Identity const& id, Data const&)>;
	using OnReadAction =
	    std::function<std::optional<Data>(uint_least16_t key, ::middleware::parameter::Identity const& id)>;

	void CreateServer(::middleware::parameter::ParameterService const& configuration);
	void CreateClients(::middleware::parameter::ParameterService const& configuration);
	void CreateSubscriptions(::middleware::parameter::ParameterService const& configuration);
	void CreatePublishers(::middleware::parameter::ParameterService const& configuration);

	// Server request handlers
	std::optional<::middleware::Reply> ProcessReadRequest(uint_least16_t key, ::middleware::Request const& request);
	std::optional<::middleware::Reply> ProcessWriteRequest(uint_least16_t key, ::middleware::Request const& request);

	// Reply formatters
	::middleware::Reply ReplyBadFormat(::middleware::Request const& request);
	::middleware::Reply ReplyNotFound(::middleware::parameter::Parameter const& request, std::string const& message);
	::middleware::Reply ReplySuccess(::middleware::parameter::Parameter const& parameter);

	bool m_connectionsLoaded{false};
	std::shared_ptr<::middleware::IMiddlewareParticipant> m_spBroadcastParticipant;
	std::shared_ptr<::middleware::IMiddlewareServer> m_spServerParticipant;
	// key=source_name value=DDS Participant
	std::map<std::string, std::shared_ptr<::middleware::IMiddlewareClient>> m_spClientParticipants{};
	// key=parameter_name value=API Handler
	std::map<std::string, std::shared_ptr<::middleware::parameter::IPublish>> m_spPublishers{};
	std::map<std::string, std::shared_ptr<::middleware::parameter::IRead>> m_spReadClients{};
	std::map<std::string, std::shared_ptr<::middleware::parameter::IWrite>> m_spWriteClients{};
	std::vector<std::unique_ptr<::middleware::parameter::IBasic>> m_spSubscribers{};

	std::map<Identity, OnWriteAction> m_onWriteActions{};
	std::map<Identity, OnReadAction> m_onReadActions{};
};
}  // namespace parameter
}  // namespace middleware
#endif  // ParameterService_H
