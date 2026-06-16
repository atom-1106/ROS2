// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: mockMiddlewareReaderStub.h
// Description: Mock class for the Middleware DataReaderStub interface.

#ifndef mockMiddlewareReaderStub_H
#define mockMiddlewareReaderStub_H

#include <gmock/gmock.h>
#include "IDataReaderStub.h"

namespace mock
{
class DataReaderStub : public ::middleware::dds::IDataReaderStub
{
public:
	virtual ~DataReaderStub() override = default;
	MOCK_METHOD(::eprosima::fastdds::dds::ReturnCode_t, TakeNextSample, (void*, ::eprosima::fastdds::dds::SampleInfo*));
};
}  // namespace mock

#endif  // mockMiddlewareReaderStub_H
