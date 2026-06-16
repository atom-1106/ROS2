// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: StubCreator.cpp
// Description: Hermes gRPC Stub factory

#include "StubCreator.h"
#include "SystemUtilities.h"

#include <grpc++/security/credentials.h>
#include <grpcpp/create_channel.h>

namespace middleware
{
namespace grpc
{
StubCreator::StubCreator(std::string const& uri, std::shared_ptr<::grpc::ChannelCredentials> const& spChannelCreds)
    : m_serverURI(uri), m_spChannelCreds(spChannelCreds)
{
	::middleware::log::Info("[StubCreator] Client initialized with URI: [%s]", m_serverURI.c_str());
}
std::unique_ptr<::cat::middleware::grpc::Transaction::Stub> StubCreator::CreateTransactionStub()
{
	CheckChannel();  // Make Sure channel created
	return ::cat::middleware::grpc::Transaction::NewStub(m_spChannel);
}
void StubCreator::AddOnDisconnect(OnDisconnectCallback&& on_disconnect)
{
	m_onDisconnectCallbacks.emplace_back(std::move(on_disconnect));
}
void StubCreator::CheckChannel()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	if(!m_spChannel || (m_spChannel->GetState(true) != GRPC_CHANNEL_READY))
	{
		m_spChannel = ::grpc::CreateChannel(m_serverURI, m_spChannelCreds);
		if(m_connectionStatus == ConnectionStatus::NOT_CONNECTED)
		{
			m_connectionStatus = ConnectionStatus::CONNECTED;
		}
		else if(m_connectionStatus == ConnectionStatus::CONNECTED)
		{
			for(auto const& on_disconnect : m_onDisconnectCallbacks)
			{
				on_disconnect();
			}
		}
	}
}
std::string StubCreator::Uri()
{
	return m_serverURI;
}
}  // namespace grpc
}  // namespace middleware
