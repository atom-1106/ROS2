// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ParameterService.cpp
// Description: Interface API for external users.

#include "ParameterService.h"
#include "MiddlewareFrameworkFactory.h"
#include "ParameterEndpointConfigurationUtilities.h"
#include "PublishParameter.h"
#include "ReadParameter.h"
#include "SubscribeParameter.h"
#include "SystemUtilities.h"
#include "WriteParameter.h"

#include <middleware_parameter.pb.h>

#include <algorithm>
#include <cassert>
#include <chrono>

namespace
{
static constexpr uint32_t n_DomainIdForReadWrite{1};
static constexpr uint32_t n_ThreadPoolSizeForReadWrite{5};
static std::chrono::seconds const n_ClientRequestTimeout{10};
}  // namespace

namespace middleware
{
namespace parameter
{
Service::Service(::middleware::parameter::ParameterService const& configuration)
    : m_connectionsLoaded{::middleware::parameter::Domain::Load(configuration)},
      m_spBroadcastParticipant(
          ::middleware::FrameworkFactory::CreateMessageParticipant(0, configuration.server_identity)
      )
{
	assert(m_spBroadcastParticipant);

	if(!m_connectionsLoaded)
	{
		::middleware::log::Warning(
		    "[ParameterService] Static connections configuration failed validation against API configuration."
		);
	}
	CreateServer(configuration);
	CreateClients(configuration);

	CreateSubscriptions(configuration);
	CreatePublishers(configuration);
}
Service::~Service()
{
	// ensure destruction / shouldStop is set before any members of this class (which may be accessed by callbacks) are destructed
	m_spServerParticipant.reset();
	for(auto& i : m_spPublishers)
	{
		i.second.reset();
	}
	for(auto& i : m_spReadClients)
	{
		i.second.reset();
	}
	for(auto& i : m_spWriteClients)
	{
		i.second.reset();
	}
	for(auto& i : m_spClientParticipants)
	{
		i.second.reset();
	}
}

::middleware::parameter::IPublish& Service::RetrievePublisherInstance(::middleware::parameter::Identity const& identity)
{
	auto found = m_spPublishers.find(identity);
	if(found == std::cend(m_spPublishers))
	{
		::middleware::log::Error("[ParameterService] Publisher [%s] was not found.", identity.c_str());
	}
	// ::middleware::log::Error throws, this won't dereference past end
	// cppcheck-suppress derefInvalidIteratorRedundantCheck
	return *found->second;
}
::middleware::parameter::IRead& Service::RetrieveReadInstance(::middleware::parameter::Identity const& identity)
{
	auto found = m_spReadClients.find(identity);
	if(found == std::cend(m_spReadClients))
	{
		::middleware::log::Error("[ParameterService] Read Client [%s] was not found.", identity.c_str());
	}
	// ::middleware::log::Error throws, this won't dereference past end
	// cppcheck-suppress derefInvalidIteratorRedundantCheck
	return *found->second;
}
::middleware::parameter::IWrite& Service::RetrieveWriteInstance(::middleware::parameter::Identity const& identity)
{
	auto found = m_spWriteClients.find(identity);
	if(found == std::cend(m_spWriteClients))
	{
		::middleware::log::Error("[ParameterService] Write Client [%s] was not found.", identity.c_str());
	}
	// ::middleware::log::Error throws, this won't dereference past end
	// cppcheck-suppress derefInvalidIteratorRedundantCheck
	return *found->second;
}
void Service::CreateServer(::middleware::parameter::ParameterService const& configuration)
{
	if(configuration.server_write_parameters.empty() && configuration.server_read_parameters.empty())
	{
		// No actions, do not create server
		::middleware::log::Info("[ParameterService] No server actions configured. Skipping server creation.");
		return;
	}
	else
	{
		for(auto const& entry : configuration.server_write_parameters)
		{
			m_onWriteActions[entry.parameter_id] = entry.callback;
		}
		for(auto const& entry : configuration.server_read_parameters)
		{
			m_onReadActions[entry.parameter_id] = entry.callback;
		}

		m_spServerParticipant = ::middleware::FrameworkFactory::CreateServerParticipant(
		    n_DomainIdForReadWrite,
		    ::middleware::parameter::Domain::FormatName(configuration.server_identity),
		    n_ThreadPoolSizeForReadWrite,
		    [this](auto key, auto const& request) { return ProcessReadRequest(key, request); },
		    [this](auto key, auto const& request) { return ProcessWriteRequest(key, request); }
		);
		m_spServerParticipant->Run();
	}
}
void Service::CreateClients(::middleware::parameter::ParameterService const& configuration)
{
	auto connections = ::middleware::parameter::Domain::GetClientConnections(configuration);
	for(auto const& source : connections)
	{
		m_spClientParticipants[source.first] = ::middleware::FrameworkFactory::CreateClientParticipant(
		    n_DomainIdForReadWrite,
		    configuration.server_identity,
		    source.first,
		    n_ThreadPoolSizeForReadWrite,
		    n_ClientRequestTimeout
		);
		for(auto const& parameter : configuration.client_read_parameters)
		{
			auto info = std::find_if(
			    std::cbegin(source.second),
			    std::cend(source.second),
			    [name = ::middleware::parameter::Domain::FormatName(parameter.parameter_id)](auto const& param_info)
			    { return param_info.parameter_name == name; }
			);
			if(info != std::cend(source.second))
			{
				m_spReadClients[parameter.parameter_id] = std::make_shared<::middleware::parameter::ReadParameter>(
				    parameter, m_spClientParticipants.at(source.first)
				);
			}
		}
		for(auto const& parameter : configuration.client_write_parameters)
		{
			auto info = std::find_if(
			    std::cbegin(source.second),
			    std::cend(source.second),
			    [name = ::middleware::parameter::Domain::FormatName(parameter.parameter_id)](auto const& param_info)
			    { return param_info.parameter_name == name; }
			);
			if(info != std::cend(source.second))
			{
				m_spWriteClients[parameter.parameter_id] = std::make_shared<::middleware::parameter::WriteParameter>(
				    parameter, m_spClientParticipants.at(source.first)
				);
			}
		}
	}
}
void Service::CreateSubscriptions(::middleware::parameter::ParameterService const& configuration)
{
	std::transform(
	    std::cbegin(configuration.subscribed_parameters),
	    std::cend(configuration.subscribed_parameters),
	    std::back_inserter(m_spSubscribers),
	    [this](auto const& entry)
	    { return std::make_unique<::middleware::parameter::SubscribeParameter>(entry, m_spBroadcastParticipant); }
	);
}
void Service::CreatePublishers(::middleware::parameter::ParameterService const& configuration)
{
	for(auto const& entry : configuration.published_parameters)
	{
		m_spPublishers[entry.parameter_id] =
		    std::make_shared<::middleware::parameter::PublishParameter>(entry, m_spBroadcastParticipant);
	}
}
void Service::Reply(uint_least16_t key, Data const& data)
{
	::middleware::Reply reply{};
	reply.status.status_code  = ::middleware::StatusCode::STATUS_CODE_SUCCESS;
	reply.serialized_protobuf = ::middleware::common::SerializeToVector(data);
	m_spServerParticipant->Reply(key, reply);
}
std::optional<::middleware::Reply> Service::ProcessReadRequest(uint_least16_t key, ::middleware::Request const& request)
{
	Parameter request_protobuf{};
	if(request_protobuf.ParseFromArray(request.serialized_protobuf.data(), request.serialized_protobuf.size()))
	{
		auto const entry = m_onReadActions.find(request_protobuf.info().identity());
		if(entry != std::cend(m_onReadActions))
		{
			::middleware::log::Debug(
			    "[ParameterServer] READ action called for [%s].", request_protobuf.info().identity().c_str()
			);

			auto response_data = entry->second(key, request_protobuf.info().identity());
			if(response_data)
			{
				Parameter reply_protobuf{};
				reply_protobuf.mutable_info()->CopyFrom(request_protobuf.info());
				reply_protobuf.mutable_data()->CopyFrom(*response_data);
				return ReplySuccess(reply_protobuf);
			}
			else
			{
				::middleware::log::Debug(
				    "[ParameterServer] READ action reply not ready for [%s].",
				    request_protobuf.info().identity().c_str()
				);
				return std::nullopt;
			}
		}
		else
		{
			std::stringstream error_msg{};
			error_msg << " READ action not found for [" << request_protobuf.info().identity() << "]";
			return ReplyNotFound(request_protobuf, error_msg.str());
		}
	}
	else
	{
		return ReplyBadFormat(request);
	}
}
std::optional<::middleware::Reply>
Service::ProcessWriteRequest(uint_least16_t key, ::middleware::Request const& request)
{
	Parameter request_protobuf{};
	if(request_protobuf.ParseFromArray(request.serialized_protobuf.data(), request.serialized_protobuf.size()))
	{
		auto const entry = m_onWriteActions.find(request_protobuf.info().identity());
		if(entry != std::cend(m_onWriteActions))
		{
			::middleware::log::Debug(
			    "[ParameterServer] WRITE action called for [%s].", request_protobuf.info().identity().c_str()
			);

			auto response_data = entry->second(key, request_protobuf.info().identity(), request_protobuf.data());
			if(response_data)
			{
				Parameter reply_protobuf{};
				reply_protobuf.mutable_info()->CopyFrom(request_protobuf.info());
				reply_protobuf.mutable_data()->CopyFrom(*response_data);
				return ReplySuccess(reply_protobuf);
			}
			else
			{
				::middleware::log::Debug(
				    "[ParameterServer] WRITE action reply not ready for [%s].",
				    request_protobuf.info().identity().c_str()
				);
				return std::nullopt;
			}
		}
		else
		{
			std::stringstream error_msg{};
			error_msg << " WRITE action not found for [" << request_protobuf.info().identity() << "]";
			return ReplyNotFound(request_protobuf, error_msg.str());
		}
	}
	else
	{
		return ReplyBadFormat(request);
	}
}
::middleware::Reply Service::ReplyBadFormat(::middleware::Request const& request)
{
	// Create
	::middleware::Reply reply{};
	reply.status.status_code    = ::middleware::StatusCode::STATUS_CODE_REQUEST_FORMAT_ERROR;
	reply.status.status_message = "Request could not be parsed.";

	::middleware::log::Warning("[ParameterServer] %s.", reply.status.status_message.c_str());
	reply.serialized_protobuf = request.serialized_protobuf;
	return reply;
}
::middleware::Reply
Service::ReplyNotFound(::middleware::parameter::Parameter const& request, std::string const& message)
{
	// Log
	::middleware::log::Warning("[ParameterServer] %s", message.c_str());
	// Create
	::middleware::Reply reply{};
	reply.status.status_code    = ::middleware::StatusCode::STATUS_CODE_REQUESTED_NOT_FOUND_FAILURE;
	reply.status.status_message = message;

	::middleware::parameter::Parameter replyProtobuf{};
	replyProtobuf.mutable_info()->CopyFrom(request.info());
	replyProtobuf.mutable_data()->set_quality(::cat::middleware::parameter::QUALITY_NOT_RECEIVED);
	::middleware::common::SerializeToVector(replyProtobuf.data(), reply.serialized_protobuf);
	return reply;
}
::middleware::Reply Service::ReplySuccess(::middleware::parameter::Parameter const& parameter)
{
	::middleware::Reply reply{};
	reply.status.status_code = ::middleware::StatusCode::STATUS_CODE_SUCCESS;
	::middleware::common::SerializeToVector(parameter.data(), reply.serialized_protobuf);
	return reply;
}
}  // namespace parameter
}  // namespace middleware
