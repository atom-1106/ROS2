// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: mockMiddlewareWriterStub.h
// Description: Mock class for the MiddlewareReader interface.

#ifndef mockMiddlewareWriterStub_H
#define mockMiddlewareWriterStub_H

#include <gmock/gmock.h>
#include "IDataWriterStub.h"

namespace mock
{
class DataWriterStub : public ::middleware::dds::IDataWriterStub
{
public:
	virtual ~DataWriterStub() override = default;
	MOCK_METHOD(::eprosima::fastdds::dds::ReturnCode_t, Write, (void*));
};
}  // namespace mock

#endif  // mockMiddlewareWriterStub_H
