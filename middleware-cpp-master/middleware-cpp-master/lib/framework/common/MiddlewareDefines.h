// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareFrameworkDefines.h
// Description: Common defined types and values for middleware framework.

#ifndef MiddlewareFrameworkDefines_H
#define MiddlewareFrameworkDefines_H

#include <chrono>
#include <cstdint>
#include <functional>
#include <future>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace middleware
{

using Bytes    = std::vector<uint8_t>;
using Identity = std::string;

namespace timeouts_ms
{
uint32_t const frameworkReadWrite = 5000;
}  // namespace timeouts_ms
namespace timeouts_ns
{
uint32_t const topicSearchDuration{10000};
}
namespace max
{
static constexpr uint32_t n_MaxPayloadByteCount = 4000000;  // 4 MB Limit
}  // namespace max

// Middleware Owned Error Values
enum class StatusCode
{
	STATUS_CODE_UNSPECIFIED,
	STATUS_CODE_SUCCESS,
	STATUS_CODE_SERVER_NOT_AVAILABLE,
	STATUS_CODE_CLIENT_TIMEOUT_ERROR,
	STATUS_CODE_SERVER_TIMEOUT_ERROR,
	STATUS_CODE_REQUESTED_NOT_FOUND_FAILURE,
	STATUS_CODE_REQUEST_FORMAT_ERROR,
	STATUS_CODE_REPLY_FORMAT_ERROR,
	STATUS_CODE_CLIENT_REQUEST_SIZE_ERROR,
	STATUS_CODE_SERVER_NO_REPLY_ERROR,
	STATUS_CODE_SERVER_INTERNAL_ERROR,
	STATUS_CODE_CLIENT_INIT_ERROR,
	STATUS_CODE_SERVER_REQUEST_SIZE_ERROR,
};

struct Request
{
	Bytes serialized_protobuf{};
};
struct Status
{
	::middleware::StatusCode status_code{::middleware::StatusCode::STATUS_CODE_UNSPECIFIED};
	std::string status_message{};
};
struct Reply
{
	::middleware::Status status{};
	Bytes serialized_protobuf{};
};
using OnRequestCallback  = std::function<std::optional<Reply>(uint64_t, Request const&)>;
using OnCompleteCallback = std::function<void(Reply const&)>;
}  // namespace middleware
#endif
