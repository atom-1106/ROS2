// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ServerRequestHandler.cpp
// Description: Server side handler implementation for client requests.

#include "ServerRequestHandler.h"
#include "SystemUtilities.h"

#include <cassert>
#include <chrono>

namespace
{
static constexpr auto n_ReplyTimeout = std::chrono::seconds(10);
}  // namespace

namespace middleware
{
namespace dds
{
ServerRequestHandler::ServerRequestHandler(
    ::middleware::OnRequestCallback&& on_reads,
    ::middleware::OnRequestCallback&& on_writes
)
    : m_readCallback{std::move(on_reads)}, m_writeCallback{std::move(on_writes)}
{
	assert(m_readCallback);
	assert(m_writeCallback);
}
void ServerRequestHandler::Reply(uint_least16_t requestKey, ::middleware::Reply const& reply)
{
	std::lock_guard<std::mutex> lock(m_replyMutex);
	auto it = m_replies.find(requestKey);
	if(it != m_replies.end())
	{
		it->second.set_value(reply);
	}
	else
	{
		::middleware::log::Warning(
		    "[ServerRequestHandler] No matching request found for reply with key [%llu].", requestKey
		);
	}
}
cat::middleware::dds::TransferData ServerRequestHandler::Read(
    eprosima::fastdds::dds::rpc::RpcRequest const& info,
    /*in*/ cat::middleware::dds::TransferData const& req
)
{
	return ProcessRequest(
	    info,
	    req,
	    [this](uint_least16_t key, ::middleware::Request const& request) { return m_readCallback(key, request); }
	);
}

cat::middleware::dds::TransferData ServerRequestHandler::Write(
    eprosima::fastdds::dds::rpc::RpcRequest const& info,
    /*in*/ cat::middleware::dds::TransferData const& req
)
{
	return ProcessRequest(
	    info,
	    req,
	    [this](uint_least16_t key, ::middleware::Request const& request) { return m_writeCallback(key, request); }
	);
}
::cat::middleware::dds::TransferData ServerRequestHandler::ProcessRequest(
    ::eprosima::fastdds::dds::rpc::RpcRequest const& info,
    ::cat::middleware::dds::TransferData const& req,
    ServerRequestHandler::ServerCallback&& callback
)
{
	ThrowOnEmptyRequest(info.get_client_id(), req);
	// Expected response
	std::optional<::middleware::Reply> callbackReply;
	std::optional<::cat::middleware::dds::TransferData> response;
	auto const currentKey = ++m_requestKeyCounter;
	try
	{
		::middleware::Request request{};
		request.serialized_protobuf = req.protobuf();
		callbackReply               = callback(currentKey, request);
	}
	catch(std::exception const& e)
	{
		ThrowOnCallbackException(info.get_client_id(), e, req);
	}
	if(callbackReply)
	{
		if(callbackReply->serialized_protobuf.size() > ::middleware::max::n_MaxPayloadByteCount)
		{
			ThrowOnResponseTooLarge(info.get_client_id(), req, callbackReply->serialized_protobuf.size());
		}
		cat::middleware::dds::TransferData data{};
		data.protobuf(callbackReply->serialized_protobuf);
		response = std::move(data);
	}
	else
	{
		// The future context
		std::future<::middleware::Reply> currentFuture;

		// Add new promise and get future
		std::unique_lock<std::mutex> lock(m_replyMutex);
		auto [it, inserted] =
		    m_replies.emplace(currentKey, std::promise<::middleware::Reply>());  // Create promise for this request
		currentFuture = it->second.get_future();
		lock.unlock();

		auto status = currentFuture.wait_for(n_ReplyTimeout);
		if(status == std::future_status::ready)
		{
			::middleware::log::Debug("[ServerRequestHandler] Write request finished within expected time.");
			auto deferredReply = currentFuture.get();
			if(deferredReply.serialized_protobuf.size() > ::middleware::max::n_MaxPayloadByteCount)
			{
				ThrowOnResponseTooLarge(info.get_client_id(), req, deferredReply.serialized_protobuf.size());
			}
			cat::middleware::dds::TransferData data{};
			data.protobuf(deferredReply.serialized_protobuf);
			response = std::move(data);
		}
		lock.lock();
		m_replies.erase(currentKey);  // Clean up promise after retrieving the response
		lock.unlock();
		if(status == std::future_status::timeout)
		{
			ThrowOnNoResponse(info.get_client_id(), req);
		}
	}
	return *response;
}

void ServerRequestHandler::ThrowOnNoResponse(
    ::eprosima::fastdds::rtps::GUID_t const clientId,
    ::cat::middleware::dds::TransferData const& req
)
{
	std::stringstream error_msg{};
	error_msg << "No response was generated for request from [" << clientId << "].";
	::middleware::log::Debug("[ServerRequestHandler] %s", error_msg.str().c_str());
	::cat::middleware::dds::RequestException exception(error_msg.str());
	exception.status_code(static_cast<uint32_t>(::middleware::StatusCode::STATUS_CODE_SERVER_NO_REPLY_ERROR));
	exception.request(req);
	throw exception;
}
void ServerRequestHandler::ThrowOnEmptyRequest(
    ::eprosima::fastdds::rtps::GUID_t const clientId,
    ::cat::middleware::dds::TransferData const& req
)
{
	if(req.protobuf().empty())
	{
		std::stringstream error_msg{};
		error_msg << "Empty request was received from [" << clientId << "].";
		::middleware::log::Debug("[ServerRequestHandler] %s", error_msg.str().c_str());
		::cat::middleware::dds::RequestException exception(error_msg.str());
		exception.status_code(static_cast<uint32_t>(::middleware::StatusCode::STATUS_CODE_CLIENT_REQUEST_SIZE_ERROR));
		exception.request(req);
		throw exception;
	}
}
void ServerRequestHandler::ThrowOnCallbackException(
    ::eprosima::fastdds::rtps::GUID_t const clientId,
    std::exception const& e,
    ::cat::middleware::dds::TransferData const& req
)
{
	std::stringstream error_msg{};
	error_msg << "Client [" << clientId << "] on callback exception: " << e.what();
	::middleware::log::Debug("[ServerRequestHandler] %s", error_msg.str().c_str());
	::cat::middleware::dds::RequestException exception(error_msg.str());
	exception.status_code(static_cast<uint32_t>(::middleware::StatusCode::STATUS_CODE_SERVER_INTERNAL_ERROR));
	exception.request(req);
	throw exception;
}
void ServerRequestHandler::ThrowOnResponseTooLarge(
    ::eprosima::fastdds::rtps::GUID_t const clientId,
    ::cat::middleware::dds::TransferData const& req,
    size_t responseSize
)
{
	std::stringstream error_msg{};
	error_msg << "Response size [" << responseSize << "] exceeded max payload ["
	          << ::middleware::max::n_MaxPayloadByteCount << "] for request from [" << clientId << "].";
	::middleware::log::Debug("[ServerRequestHandler] %s", error_msg.str().c_str());
	::cat::middleware::dds::RequestException exception(error_msg.str());
	exception.status_code(static_cast<uint32_t>(::middleware::StatusCode::STATUS_CODE_SERVER_REQUEST_SIZE_ERROR));
	exception.request(req);
	throw exception;
}

}  // namespace dds
}  // namespace middleware
