// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: mockWriteParameter.h
// Description: Mock class for the WriteParameter interface.

#ifndef mockWriteParameter_H
#define mockWriteParameter_H

#include <gmock/gmock.h>
#include "IWriteParameter.h"

namespace mock
{
class WriteParameter : public ::middleware::parameter::IWrite
{
public:
	virtual ~WriteParameter() override = default;
	MOCK_METHOD(void, Write, (::middleware::parameter::Data const&, ::middleware::parameter::IWrite::OnCompletion&&));
	MOCK_METHOD(::middleware::parameter::Identity, Identity, ());
	MOCK_METHOD(::middleware::parameter::Unit, Unit, ());
};
}  // namespace mock

#endif  // mockReadParameter_H
