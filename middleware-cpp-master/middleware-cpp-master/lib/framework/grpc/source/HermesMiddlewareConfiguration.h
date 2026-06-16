// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: HermesMiddlewareConfiguration.h
// Description: Hermes creation factory

#ifndef HermesMiddlewareConfiguration_H
#define HermesMiddlewareConfiguration_H

#include <grpc++/security/credentials.h>
#include <grpc++/security/server_credentials.h>
#include "DefinedTypes.h"
#include "MiddlewareDefines.h"

namespace middleware
{
namespace config
{
struct Client
{
	std::string client_name{};
	std::string ip_address{"127.0.0.1"};
	uint32_t port{50000};
	std::shared_ptr<::grpc::ChannelCredentials> spCredentials = ::grpc::InsecureChannelCredentials();
};
struct Server
{
	std::string ip_address{"127.0.0.1"};
	uint32_t port{50000};
	std::shared_ptr<::grpc::ServerCredentials> spCredentials = ::grpc::InsecureServerCredentials();
};
struct Publisher
{
	std::string connection_name{};
	::middleware::HistoryType history_type{::middleware::HistoryType::LAST_1};
};
struct Subscriber
{
	std::string connection_name{};
	::middleware::HistoryType history_type{::middleware::HistoryType::LAST_1};
};
}  // namespace config
}  // namespace middleware
#endif  // HermesMiddlewareConfiguration_H
