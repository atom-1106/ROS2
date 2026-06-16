// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareServer.cpp
// Description: Implementation for DDS transactional server.

#include "MiddlewareServer.h"
#include "SystemUtilities.h"

#include <cassert>

namespace middleware
{
namespace dds
{
Server::Server(std::string const& name, std::unique_ptr<::middleware::dds::IServerRpcStub>&& spServerStub)
    : m_name{name}, m_spServerStub{std::move(spServerStub)}
{
	assert(m_spServerStub);
}
Server::~Server()
{
	Stop();
}
void Server::Run()
{
	m_thread = std::jthread(
	    [this]
	    {
		    ::middleware::log::Info("[MiddlewareServer][%s] Server is running.", m_name.c_str());
		    m_spServerStub->Run();
		    ::middleware::log::Info("[MiddlewareServer][%s] Server was stopped.", m_name.c_str());
	    }
	);
}
void Server::Stop()
{
	::middleware::log::Info("[MiddlewareServer][%s] Server is stopping.", m_name.c_str());
	m_spServerStub->Stop();
}
void Server::Reply(uint_least16_t key, ::middleware::Reply const& reply)
{
	m_spServerStub->Reply(key, reply);
}
}  // namespace dds
}  // namespace middleware
