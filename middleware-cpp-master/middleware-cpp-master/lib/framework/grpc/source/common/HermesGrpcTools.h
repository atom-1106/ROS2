// COPYRIGHT (C) CATERPILLAR INC. ALL RIGHTS RESERVED.
// File: HermesGrpcTools.h
// Description: Common defined types for Hermes middleware

#ifndef HermesGrpcTools_H
#define HermesGrpcTools_H

#include <grpcpp/support/status.h>
#include "DefinedTypes.h"
#include "MiddlewareDefines.h"

namespace middleware
{
namespace grpc
{
class tools
{
public:
	virtual ~tools() = default;
	static ::middleware::Request FromRequest(GrpcRequest const& request);
	static GrpcRequest ToRequest(::middleware::Request const& request);
	static ::middleware::Reply FromReply(GrpcReply const& reply);
	static GrpcReply ToReply(::middleware::Reply const& reply);
	static ::middleware::StatusCode FromStatus(GrpcReplyStatusCode const code);
	static GrpcReplyStatusCode ToStatus(::middleware::StatusCode const code);
	static ::middleware::RequestType FromType(GrpcRequestType const type);
	static GrpcRequestType ToType(::middleware::RequestType const type);
	static GrpcReply ReplyTimeout(::middleware::Request const& request);
	static ::middleware::Reply ReplyServerNotAvailable(GrpcRequest const& request);
	static ::middleware::Reply ReplyNotOk(::grpc::Status status);
};
}  // namespace grpc
}  // namespace middleware
#endif  // MiddlewareDdsUtilities_H
