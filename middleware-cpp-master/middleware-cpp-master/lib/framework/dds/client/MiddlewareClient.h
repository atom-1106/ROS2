// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareClient.h
// Description: Implementation for DDS transactional client.

#ifndef MiddlewareClient_DDS_H
#define MiddlewareClient_DDS_H

#include "IMiddlewareClient.h"

#include "IClientRpcStub.h"
#include "IMiddlewareClient.h"
#include "IThreadPool.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace middleware
{
namespace dds
{
class Client final : public ::middleware::IMiddlewareClient
{
public:
	Client(
	    std::string const& name,
	    std::chrono::milliseconds const timeout_ms,
	    std::unique_ptr<::middleware::dds::IClientRpcStub>&& spClientStub,
	    std::unique_ptr<::middleware::IThreadPool>&& spThreadPool
	);
	virtual ~Client() override;
	void Read(::google::protobuf::Message const& message, ::middleware::OnCompleteCallback&& on_complete) override;
	void Write(::google::protobuf::Message const& message, ::middleware::OnCompleteCallback&& on_complete) override;

private:
	bool RejectIfOversized(
	    std::vector<uint8_t> const& serialized,
	    ::middleware::OnCompleteCallback const& on_complete
	) const;

	std::string const m_name{};
	std::chrono::milliseconds const m_timeout{100};
	std::unique_ptr<::middleware::dds::IClientRpcStub> const m_spClientStub;
	std::unique_ptr<::middleware::IThreadPool> const m_spThreadPool;
};
}  // namespace dds
}  // namespace middleware
#endif  // MiddlewareClient_DDS_H
