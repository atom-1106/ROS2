// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: mockMiddlewareWriter.h
// Description: Mock class for the MiddlewareWriter interface.

#ifndef mockMiddlewareWriter_H
#define mockMiddlewareWriter_H

#include <gmock/gmock.h>
#include "IMiddlewareWriter.h"

namespace mock
{
class MiddlewareWriter : public ::middleware::IMiddlewareWriter
{
public:
	virtual ~MiddlewareWriter() override = default;
	MOCK_METHOD(bool, ConnectedToSubscriber, ());
	MOCK_METHOD(bool, Publish, (::google::protobuf::Message const&));
	MOCK_METHOD(bool, Publish, (std::vector<uint8_t> const&));
	MOCK_METHOD(void, OnPublicationMatched, (bool));
};
}  // namespace mock

#endif  // mockMiddlewareWriter_H
