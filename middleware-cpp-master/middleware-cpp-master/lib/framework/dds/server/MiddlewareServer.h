// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareServer.h
// Description: Implementation for DDS transactional server.

#ifndef MiddlewareServer_DDS_H
#define MiddlewareServer_DDS_H

// Middleware
#include "IMiddlewareServer.h"
#include "IServerRpcStub.h"

#include <atomic>
#include <memory>
#include <string>
#include <thread>

namespace middleware
{
namespace dds
{
class Server final : public ::middleware::IMiddlewareServer
{
public:
	Server(std::string const& name, std::unique_ptr<::middleware::dds::IServerRpcStub>&& spServerStub);
	virtual ~Server() override;
	void Run() override;
	void Stop() override;
	void Reply(uint_least16_t key, ::middleware::Reply const& reply) override;

private:
	std::string const m_name{};
	std::unique_ptr<::middleware::dds::IServerRpcStub> const m_spServerStub;
	std::jthread m_thread;
};
}  // namespace dds
}  // namespace middleware
#endif  // MiddlewareServer_DDS_H
