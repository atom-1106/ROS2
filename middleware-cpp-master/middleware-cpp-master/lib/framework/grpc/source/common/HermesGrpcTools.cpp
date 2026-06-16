// COPYRIGHT (C) CATERPILLAR INC. ALL RIGHTS RESERVED.
// File: HermesGrpcTools.cpp
// Description: Common defined types for Hermes middleware

#include "HermesGrpcTools.h"
#include <cassert>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace middleware
{
namespace grpc
{
::middleware::Request tools::FromRequest(GrpcRequest const& request)
{
	::middleware::Request converted{};
	converted.client_id = request.client_id();
	converted.type      = FromType(request.type());
	converted.serialized_protobuf =
	    std::vector<uint8_t>(std::cbegin(request.serialized_protobuf()), std::cend(request.serialized_protobuf()));
	return converted;
}
GrpcRequest tools::ToRequest(::middleware::Request const& request)
{
	GrpcRequest converted{};
	converted.set_client_id(request.client_id);
	converted.set_type(ToType(request.type));
	converted.set_serialized_protobuf(
	    std::string(std::cbegin(request.serialized_protobuf), std::cend(request.serialized_protobuf))
	);
	return converted;
}
::middleware::Reply tools::FromReply(GrpcReply const& reply)
{
	::middleware::Reply converted{};
	converted.client_id             = reply.client_id();
	converted.status.status_code    = FromStatus(reply.status().status_code());
	converted.status.status_message = reply.status().status_message();
	converted.serialized_protobuf =
	    std::vector<uint8_t>(std::cbegin(reply.serialized_protobuf()), std::cend(reply.serialized_protobuf()));
	return converted;
}
GrpcReply tools::ToReply(::middleware::Reply const& reply)
{
	GrpcReply converted{};
	converted.set_client_id(reply.client_id);
	converted.mutable_status()->set_status_code(ToStatus(reply.status.status_code));
	converted.mutable_status()->set_status_message(reply.status.status_message);
	converted.set_serialized_protobuf(
	    std::string(std::cbegin(reply.serialized_protobuf), std::cend(reply.serialized_protobuf))
	);
	return converted;
}
::middleware::StatusCode tools::FromStatus(GrpcReplyStatusCode const code)
{
	switch(code)
	{
		case GrpcReplyStatus::STATUS_CODE_SUCCESS: return ::middleware::StatusCode::STATUS_CODE_SUCCESS;
		case GrpcReplyStatus::STATUS_CODE_STANDARD_FAILURE:
			return ::middleware::StatusCode::STATUS_CODE_STANDARD_FAILURE;
		case GrpcReplyStatus::STATUS_CODE_REQUESTED_NOT_FOUND_FAILURE:
			return ::middleware::StatusCode::STATUS_CODE_REQUESTED_NOT_FOUND_FAILURE;
		case GrpcReplyStatus::STATUS_CODE_EMPTY_REQUEST_FAILURE:
			return ::middleware::StatusCode::STATUS_CODE_EMPTY_REQUEST_FAILURE;
		case GrpcReplyStatus::STATUS_CODE_SERVER_NOT_AVAILABLE:
			return ::middleware::StatusCode::STATUS_CODE_SERVER_NOT_AVAILABLE;
		case GrpcReplyStatus::STATUS_CODE_NOT_AUTHORIZED_ERROR:
			return ::middleware::StatusCode::STATUS_CODE_NOT_AUTHORIZED_ERROR;
		case GrpcReplyStatus::STATUS_CODE_TIMEOUT_ERROR: return ::middleware::StatusCode::STATUS_CODE_TIMEOUT_ERROR;
		case GrpcReplyStatus::STATUS_CODE_REQUEST_FORMAT_ERROR:
			return ::middleware::StatusCode::STATUS_CODE_REQUEST_FORMAT_ERROR;
		case GrpcReplyStatus::STATUS_CODE_SERVER_INTERNAL_ERROR:
			return ::middleware::StatusCode::STATUS_CODE_SERVER_INTERNAL_ERROR;
		case GrpcReplyStatus::STATUS_CODE_CLIENT_REQUEST_SIZE_ERROR:
			return ::middleware::StatusCode::STATUS_CODE_CLIENT_REQUEST_SIZE_ERROR;
		case GrpcReplyStatus::STATUS_CODE_SERVER_NO_REPLY_ERROR:
			return ::middleware::StatusCode::STATUS_CODE_SERVER_NO_REPLY_ERROR;
		default: return ::middleware::StatusCode::STATUS_CODE_UNSPECIFIED;
	}
}
GrpcReplyStatusCode tools::ToStatus(::middleware::StatusCode const code)
{
	switch(code)
	{
		case ::middleware::StatusCode::STATUS_CODE_SUCCESS: return GrpcReplyStatus::STATUS_CODE_SUCCESS;
		case ::middleware::StatusCode::STATUS_CODE_STANDARD_FAILURE:
			return GrpcReplyStatus::STATUS_CODE_STANDARD_FAILURE;
		case ::middleware::StatusCode::STATUS_CODE_REQUESTED_NOT_FOUND_FAILURE:
			return GrpcReplyStatus::STATUS_CODE_REQUESTED_NOT_FOUND_FAILURE;
		case ::middleware::StatusCode::STATUS_CODE_EMPTY_REQUEST_FAILURE:
			return GrpcReplyStatus::STATUS_CODE_EMPTY_REQUEST_FAILURE;
		case ::middleware::StatusCode::STATUS_CODE_SERVER_NOT_AVAILABLE:
			return GrpcReplyStatus::STATUS_CODE_SERVER_NOT_AVAILABLE;
		case ::middleware::StatusCode::STATUS_CODE_NOT_AUTHORIZED_ERROR:
			return GrpcReplyStatus::STATUS_CODE_NOT_AUTHORIZED_ERROR;
		case ::middleware::StatusCode::STATUS_CODE_TIMEOUT_ERROR: return GrpcReplyStatus::STATUS_CODE_TIMEOUT_ERROR;
		case ::middleware::StatusCode::STATUS_CODE_REQUEST_FORMAT_ERROR:
			return GrpcReplyStatus::STATUS_CODE_REQUEST_FORMAT_ERROR;
		case ::middleware::StatusCode::STATUS_CODE_SERVER_INTERNAL_ERROR:
			return GrpcReplyStatus::STATUS_CODE_SERVER_INTERNAL_ERROR;
		case ::middleware::StatusCode::STATUS_CODE_CLIENT_REQUEST_SIZE_ERROR:
			return GrpcReplyStatus::STATUS_CODE_CLIENT_REQUEST_SIZE_ERROR;
		case ::middleware::StatusCode::STATUS_CODE_SERVER_NO_REPLY_ERROR:
			return GrpcReplyStatus::STATUS_CODE_SERVER_NO_REPLY_ERROR;
		default: return GrpcReplyStatus::STATUS_CODE_UNSPECIFIED;
	}
}
::middleware::RequestType tools::FromType(GrpcRequestType const type)
{
	switch(type)
	{
		case GrpcRequest::TYPE_ALIVE: return ::middleware::RequestType::REQUEST_TYPE_ALIVE;
		case GrpcRequest::TYPE_READ: return ::middleware::RequestType::REQUEST_TYPE_READ;
		case GrpcRequest::TYPE_WRITE: return ::middleware::RequestType::REQUEST_TYPE_WRITE;
		default: return ::middleware::RequestType::REQUEST_TYPE_UNSPECIFIED;
	}
}
GrpcRequestType tools::ToType(::middleware::RequestType const type)
{
	switch(type)
	{
		case ::middleware::RequestType::REQUEST_TYPE_ALIVE: return GrpcRequest::TYPE_ALIVE;
		case ::middleware::RequestType::REQUEST_TYPE_READ: return GrpcRequest::TYPE_READ;
		case ::middleware::RequestType::REQUEST_TYPE_WRITE: return GrpcRequest::TYPE_WRITE;
		default: return GrpcRequest::TYPE_UNSPECIFIED;
	}
}
GrpcReply tools::ReplyTimeout(::middleware::Request const& request)
{
	GrpcReply reply;
	std::stringstream msg{};
	msg << "Timeout occurred for requested client_id [" << request.client_id << "]";
	reply.set_client_id(request.client_id);
	reply.mutable_status()->set_status_code(GrpcReplyStatus::STATUS_CODE_TIMEOUT_ERROR);
	reply.mutable_status()->set_status_message(msg.str());
	reply.set_serialized_protobuf(
	    std::string(std::cbegin(request.serialized_protobuf), std::cend(request.serialized_protobuf))
	);
	return reply;
}
::middleware::Reply tools::ReplyServerNotAvailable(GrpcRequest const& request)
{
	::middleware::Reply reply{};
	reply.client_id          = request.client_id();
	reply.status.status_code = ::middleware::StatusCode::STATUS_CODE_SERVER_NOT_AVAILABLE;
	std::stringstream msg{};
	msg << "Request for [" << reply.client_id << "] failed to send due to [STATUS_CODE_SERVER_NOT_AVAILABLE]";
	reply.status.status_message = msg.str();
	reply.serialized_protobuf =
	    std::vector<uint8_t>(std::cbegin(request.serialized_protobuf()), std::cend(request.serialized_protobuf()));
	return reply;
}
::middleware::Reply tools::ReplyNotOk(::grpc::Status status)
{
	::middleware::Reply reply{};
	switch(status.error_code())
	{
		case ::grpc::StatusCode::DEADLINE_EXCEEDED:
			reply.status.status_code = ::middleware::StatusCode::STATUS_CODE_TIMEOUT_ERROR;
			break;
		case ::grpc::StatusCode::NOT_FOUND:
			reply.status.status_code = ::middleware::StatusCode::STATUS_CODE_REQUESTED_NOT_FOUND_FAILURE;
			break;
		case ::grpc::StatusCode::PERMISSION_DENIED:
		case ::grpc::StatusCode::UNAUTHENTICATED:
			reply.status.status_code = ::middleware::StatusCode::STATUS_CODE_NOT_AUTHORIZED_ERROR;
			break;
		case ::grpc::StatusCode::INVALID_ARGUMENT:
		case ::grpc::StatusCode::OUT_OF_RANGE:
			reply.status.status_code = ::middleware::StatusCode::STATUS_CODE_REQUEST_FORMAT_ERROR;
			break;
		case ::grpc::StatusCode::UNAVAILABLE:
		case ::grpc::StatusCode::CANCELLED:
		case ::grpc::StatusCode::ABORTED:
			reply.status.status_code = ::middleware::StatusCode::STATUS_CODE_SERVER_NOT_AVAILABLE;
			break;
		case ::grpc::StatusCode::INTERNAL:
		case ::grpc::StatusCode::FAILED_PRECONDITION:
			reply.status.status_code = ::middleware::StatusCode::STATUS_CODE_SERVER_INTERNAL_ERROR;
			break;
		default: reply.status.status_code = ::middleware::StatusCode::STATUS_CODE_STANDARD_FAILURE; break;
	}
	reply.status.status_message = status.error_message();
	return reply;
}
}  // namespace grpc
}  // namespace middleware
