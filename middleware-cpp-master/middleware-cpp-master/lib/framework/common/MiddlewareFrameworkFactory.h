// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareFrameworkFactory.h
// Description: Middleware Framework creation factory.

#ifndef MiddlewareFrameworkFactory_H
#define MiddlewareFrameworkFactory_H

#include "IMiddlewareClient.h"
#include "IMiddlewareParticipant.h"
#include "IMiddlewareServer.h"
#include "MiddlewareDefines.h"

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace middleware
{
class FrameworkFactory
{
public:
	// Expectation of Use:
	//          Created Publishers and Servers are expected to be the Owners of the information passed.
	//          Clients who send information are relinquishing their ownership to the Server.
	virtual ~FrameworkFactory() = default;
	static std::unique_ptr<::middleware::IMiddlewareParticipant>
	CreateMessageParticipant(uint32_t domain_id, std::string const& name);

	static std::unique_ptr<::middleware::IMiddlewareClient> CreateClientParticipant(
	    uint32_t domain_id,
	    std::string const& name,
	    std::string const& service,
	    uint32_t thread_pool_size,
	    std::chrono::milliseconds const timeout
	);
	static std::unique_ptr<::middleware::IMiddlewareServer> CreateServerParticipant(
	    uint32_t domain_id,
	    std::string const& name,
	    uint32_t thread_pool_size,
	    ::middleware::OnRequestCallback&& on_reads,
	    ::middleware::OnRequestCallback&& on_writes
	);
};
}  // namespace middleware

#endif  // MiddlewareFactory_H
