// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareFrameworkFactory.cpp
// Description: Middleware Framework creation factory.

#include "MiddlewareFrameworkContext.h"
#include "MiddlewareFrameworkFactory.h"
#include "mockMiddlewareClient.h"
#include "mockMiddlewareParticipant.h"
#include "mockMiddlewareServer.h"

namespace middleware
{
std::unique_ptr<::middleware::IMiddlewareParticipant>
FrameworkFactory::CreateMessageParticipant(uint32_t const domain_id, std::string const& connection)
{
	return ::test::MiddlewareFrameworkContext::Get().TakeParticipant();
}

std::unique_ptr<::middleware::IMiddlewareClient> FrameworkFactory::CreateClientParticipant(
    uint32_t const domain_id,
    std::string const& name,
    std::string const& service,
    uint32_t const thread_pool_size,
    std::chrono::milliseconds const timeout
)
{
	return ::test::MiddlewareFrameworkContext::Get().TakeClient();
}
std::unique_ptr<::middleware::IMiddlewareServer> FrameworkFactory::CreateServerParticipant(
    uint32_t const domain_id,
    std::string const& name,
    uint32_t const thread_pool_size,
    ::middleware::OnRequestCallback&& on_reads,
    ::middleware::OnRequestCallback&& on_writes
)
{
	return ::test::MiddlewareFrameworkContext::Get().TakeServer();
}
}  // namespace middleware
