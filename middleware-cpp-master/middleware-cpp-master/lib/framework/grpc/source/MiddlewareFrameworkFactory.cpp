// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: HermesFrameworkFactory.cpp
// Description: Middleware Framework creation factory.

#include "MiddlewareFrameworkFactory.h"
#include "HermesGrpcClient.h"
#include "MiddlewareServer.h"
#include "StubCreator.h"
#include "SystemUtilities.h"

namespace
{
}

namespace middleware
{
std::unique_ptr<::middleware::IMiddlewareParticipant>
FrameworkFactory::CreateMessageParticipant(uint32_t const domain_id, std::string const& name)
{
	return nullptr;
}
std::unique_ptr<::middleware::IMiddlewareClient> FrameworkFactory::CreateClientParticipant(
    uint32_t const domain_id,
    std::string const& name,
    std::vector<std::string> const& sources
)
{
	return nullptr;
}
std::unique_ptr<::middleware::IMiddlewareServer> FrameworkFactory::CreateServerParticipant(
    uint32_t const domain_id,
    std::string const& connection,
    ::middleware::IMiddlewareServer::OnRequest&& callback
)
{
	return nullptr;
}
}  // namespace middleware
