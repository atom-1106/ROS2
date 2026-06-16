// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareClient.cpp
// Description: Implementation for DDS transactional client.

#include "MiddlewareClient.h"
#include "MiddlewareDefines.h"
#include "SystemUtilities.h"

#include <cassert>
#include <sstream>

namespace middleware
{
namespace dds
{
Client::Client(
    std::string const& name,
    std::chrono::milliseconds const timeout_ms,
    std::unique_ptr<::middleware::dds::IClientRpcStub>&& spClientStub,
    std::unique_ptr<::middleware::IThreadPool>&& spThreadPool
)
    : m_name{name},
      m_timeout{timeout_ms},
      m_spClientStub{std::move(spClientStub)},
      m_spThreadPool{std::move(spThreadPool)}
{
	assert(m_spClientStub);
	assert(m_spThreadPool);
}
Client::~Client()
{
	m_spThreadPool->Stop();
}
bool Client::RejectIfOversized(
    std::vector<uint8_t> const& serialized,
    ::middleware::OnCompleteCallback const& on_complete
) const
{
	if(serialized.size() <= ::middleware::max::n_MaxPayloadByteCount)
	{
		return false;
	}
	std::stringstream warning_message;
	warning_message << "[Client] Content exceeded " << ::middleware::max::n_MaxPayloadByteCount << " bytes.";
	::middleware::log::Warning(warning_message.str());
	::middleware::Reply reply{};
	reply.status.status_code    = ::middleware::StatusCode::STATUS_CODE_CLIENT_REQUEST_SIZE_ERROR;
	reply.status.status_message = warning_message.str();
	reply.serialized_protobuf   = serialized;
	on_complete(std::move(reply));
	return true;
}
void Client::Read(::google::protobuf::Message const& message, ::middleware::OnCompleteCallback&& on_complete)
{
	auto serialized = ::middleware::common::SerializeToVector(message);
	if(RejectIfOversized(serialized, on_complete))
	{
		return;
	}
	m_spThreadPool->Add(
	    [name       = m_name,
	     timeout    = m_timeout,
	     serialized = std::move(serialized),
	     callback   = std::move(on_complete),
	     wpClient   = m_spClientStub->GetInstance()]
	    {
		    // TODO: Set status code
		    ::middleware::Reply reply{};
		    reply.serialized_protobuf = serialized;

		    if(auto spClient = wpClient.lock())
		    {
			    ::cat::middleware::dds::TransferData request{};
			    request.protobuf(serialized);

			    auto future = spClient->Read(request);
			    if(future.wait_for(timeout) != std::future_status::ready)
			    {
				    reply.status.status_code = ::middleware::StatusCode::STATUS_CODE_CLIENT_TIMEOUT_ERROR;
			    }
			    else
			    {
				    try
				    {
					    reply.status.status_code  = ::middleware::StatusCode::STATUS_CODE_SUCCESS;
					    reply.serialized_protobuf = future.get().protobuf();
				    }
				    catch(::eprosima::fastdds::dds::rpc::RpcException const& e)
				    {
					    reply.status.status_code    = ::middleware::StatusCode::STATUS_CODE_SERVER_INTERNAL_ERROR;
					    reply.status.status_message = e.what();
				    }
			    }
		    }
		    else
		    {
			    reply.status.status_code = ::middleware::StatusCode::STATUS_CODE_CLIENT_INIT_ERROR;
		    }
		    callback(reply);
	    }
	);
}
void Client::Write(::google::protobuf::Message const& message, ::middleware::OnCompleteCallback&& on_complete)
{
	auto serialized = ::middleware::common::SerializeToVector(message);
	if(RejectIfOversized(serialized, on_complete))
	{
		return;
	}
	m_spThreadPool->Add(
	    [name       = m_name,
	     timeout    = m_timeout,
	     serialized = std::move(serialized),
	     callback   = std::move(on_complete),
	     wpClient   = m_spClientStub->GetInstance()]
	    {
		    // TODO: Set status code
		    ::middleware::Reply reply{};
		    reply.serialized_protobuf = serialized;

		    if(auto spClient = wpClient.lock())
		    {
			    ::cat::middleware::dds::TransferData request{};
			    request.protobuf(serialized);

			    auto future = spClient->Write(request);
			    if(future.wait_for(timeout) != std::future_status::ready)
			    {
				    reply.status.status_code = ::middleware::StatusCode::STATUS_CODE_CLIENT_TIMEOUT_ERROR;
			    }
			    else
			    {
				    try
				    {
					    reply.status.status_code  = ::middleware::StatusCode::STATUS_CODE_SUCCESS;
					    reply.serialized_protobuf = future.get().protobuf();
				    }
				    catch(::eprosima::fastdds::dds::rpc::RpcException const& e)
				    {
					    reply.status.status_code    = ::middleware::StatusCode::STATUS_CODE_SERVER_INTERNAL_ERROR;
					    reply.status.status_message = e.what();
				    }
			    }
		    }
		    else
		    {
			    reply.status.status_code = ::middleware::StatusCode::STATUS_CODE_CLIENT_INIT_ERROR;
		    }
		    callback(reply);
	    }
	);
}

}  // namespace dds
}  // namespace middleware
