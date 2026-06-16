// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: mockReadParameter.h
// Description: Mock class for the ReadParameter interface.

#ifndef mockReadParameter_H
#define mockReadParameter_H

#include <gmock/gmock.h>
#include "IReadParameter.h"

namespace mock
{
class ReadParameter : public ::middleware::parameter::IRead
{
public:
	virtual ~ReadParameter() override = default;
	MOCK_METHOD(void, Read, (::middleware::parameter::IRead::OnRequest&&));
	MOCK_METHOD(::middleware::parameter::Identity, Identity, ());
	MOCK_METHOD(::middleware::parameter::Unit, Unit, ());
};
}  // namespace mock

#endif  // mockReadParameter_H
