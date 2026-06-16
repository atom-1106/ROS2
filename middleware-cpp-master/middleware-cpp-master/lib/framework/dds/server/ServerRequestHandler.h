// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ServerRequestHandler.h
// Description: Server side handler implementation for client requests.

#ifndef ServerRequestHandler_H
#define ServerRequestHandler_H

#include "IAsyncRequest.h"
#include "MiddlewareDefines.h"

#include <functional>
#include <future>
#include <map>
#include <optional>

namespace middleware
{
namespace dds
{
class ServerRequestHandler final : public ::middleware::dds::IAsyncRequest
{
public:
	ServerRequestHandler(::middleware::OnRequestCallback&& on_reads, ::middleware::OnRequestCallback&& on_writes);
	virtual ~ServerRequestHandler() override = default;
	void Reply(uint_least16_t requestKey, ::middleware::Reply const& reply) override;
	cat::middleware::dds::TransferData Read(
	    eprosima::fastdds::dds::rpc::RpcRequest const& info,
	    /*in*/ cat::middleware::dds::TransferData const& req
	) override;

	cat::middleware::dds::TransferData Write(
	    eprosima::fastdds::dds::rpc::RpcRequest const& info,
	    /*in*/ cat::middleware::dds::TransferData const& req
	) override;

private:
	using ServerCallback =
	    std::function<std::optional<::middleware::Reply>(uint_least16_t, ::middleware::Request const&)>;
	::cat::middleware::dds::TransferData ProcessRequest(
	    ::eprosima::fastdds::dds::rpc::RpcRequest const& info,
	    ::cat::middleware::dds::TransferData const& req,
	    ServerCallback&& callback
	);
	void ThrowOnEmptyRequest(
	    ::eprosima::fastdds::rtps::GUID_t const clientId,
	    ::cat::middleware::dds::TransferData const& req
	);
	[[noreturn]] void ThrowOnNoResponse(
	    ::eprosima::fastdds::rtps::GUID_t const clientId,
	    ::cat::middleware::dds::TransferData const& req
	);
	[[noreturn]] void ThrowOnCallbackException(
	    ::eprosima::fastdds::rtps::GUID_t const clientId,
	    std::exception const& e,
	    ::cat::middleware::dds::TransferData const& req
	);
	[[noreturn]] void ThrowOnResponseTooLarge(
	    ::eprosima::fastdds::rtps::GUID_t const clientId,
	    ::cat::middleware::dds::TransferData const& req,
	    size_t responseSize
	);
	::middleware::OnRequestCallback m_readCallback;
	::middleware::OnRequestCallback m_writeCallback;

	std::mutex m_replyMutex;
	std::atomic<uint_least16_t> m_requestKeyCounter{0};
	std::map<uint_least16_t, std::promise<::middleware::Reply>> m_replies;
};
}  // namespace dds
}  // namespace middleware

#endif /* ServerRequestHandler_H */
