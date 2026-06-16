// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IDataReaderStub.h
// Description: Stub interface for third-party library content control.

#ifndef IDataReaderStub_H
#define IDataReaderStub_H

#include <fastdds/dds/core/detail/DDSReturnCode.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

namespace middleware
{
namespace dds
{
class IDataReaderStub
{
public:
	virtual ~IDataReaderStub() = default;
	virtual ::eprosima::fastdds::dds::ReturnCode_t
	TakeNextSample(void* pMessage, ::eprosima::fastdds::dds::SampleInfo* pInfo) = 0;
};
}  // namespace dds
}  // namespace middleware

#endif /* IDataReaderStub_H */
