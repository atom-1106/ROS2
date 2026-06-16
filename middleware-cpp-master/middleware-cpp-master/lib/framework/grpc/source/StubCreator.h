// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: StubCreator.h
// Description: Hermes gRPC Stub factory

#ifndef StubCreator_H
#define StubCreator_H

#include <grpc++/security/credentials.h>
#include "IStubCreator.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

namespace grpc
{
class Channel;
class ChannelCredentials;
}  // namespace grpc
namespace middleware
{
namespace grpc
{
class StubCreator final : public ::middleware::grpc::IStubCreator
{
public:
	StubCreator(
	    std::string const& uri,
	    std::shared_ptr<::grpc::ChannelCredentials> const& spChannelCreds = ::grpc::InsecureChannelCredentials()
	);
	virtual ~StubCreator() override = default;
	std::unique_ptr<::cat::middleware::grpc::Transaction::Stub> CreateTransactionStub() override;
	void AddOnDisconnect(::middleware::grpc::IStubCreator::OnDisconnectCallback&& on_disconnect) override;
	std::string Uri() override;

private:
	enum class ConnectionStatus
	{
		NOT_CONNECTED,
		CONNECTED,
		LOST_CONNECTION,
	};
	void CheckChannel();

	std::string m_serverURI{};
	std::shared_ptr<::grpc::ChannelCredentials> m_spChannelCreds;
	std::shared_ptr<::grpc::Channel> m_spChannel;
	std::vector<::middleware::grpc::IStubCreator::OnDisconnectCallback> m_onDisconnectCallbacks;
	ConnectionStatus m_connectionStatus{ConnectionStatus::NOT_CONNECTED};
	std::mutex m_mutex;
};
}  // namespace grpc
}  // namespace middleware
#endif  // StubCreator_H
