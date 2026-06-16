// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IDataWriterStub.h
// Description: Stub interface for third-party library content control.

#ifndef IDataWriterStub_H
#define IDataWriterStub_H

#include <fastdds/dds/core/detail/DDSReturnCode.hpp>

namespace middleware
{
namespace dds
{
class IDataWriterStub
{
public:
	virtual ~IDataWriterStub()                                           = default;
	virtual ::eprosima::fastdds::dds::ReturnCode_t Write(void* pMessage) = 0;
};
}  // namespace dds
}  // namespace middleware

#endif /* IDataWriterStub_H */
