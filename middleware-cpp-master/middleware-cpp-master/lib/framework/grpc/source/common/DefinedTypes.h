// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: DefinedTypes.h
// Description: Defined types from the Fast DDS library

#ifndef DefinedTypes_H
#define DefinedTypes_H

#include <map>
#include <memory>
#include "StubCreator.h"
//#include "middleware-framework.pb.h"

namespace middleware
{
namespace grpc
{
static std::map<uint32_t, std::shared_ptr<::middleware::grpc::IStubCreator>> n_DomainClientStubs{
    {0x00000001, std::make_shared<::middleware::grpc::StubCreator>("127.0.0.1:50001")},
    {0x00000002, std::make_shared<::middleware::grpc::StubCreator>("127.0.0.1:50002")},
    {0x00000003, std::make_shared<::middleware::grpc::StubCreator>("127.0.0.1:50003")},
    {0x00000004, std::make_shared<::middleware::grpc::StubCreator>("127.0.0.1:50004")},
    {0x00000005, std::make_shared<::middleware::grpc::StubCreator>("127.0.0.1:50005")},
};

// Grpc Types
using GrpcRequest         = ::cat::middleware::grpc::Request;
using GrpcReply           = ::cat::middleware::grpc::Reply;
using GrpcReplyStatus     = ::cat::middleware::grpc::Status;
using GrpcReplyStatusCode = ::cat::middleware::grpc::Status::Code;
using GrpcRequestType     = ::cat::middleware::grpc::Request::Type;

}  // namespace grpc
}  // namespace middleware

#endif  // DefinedTypes
