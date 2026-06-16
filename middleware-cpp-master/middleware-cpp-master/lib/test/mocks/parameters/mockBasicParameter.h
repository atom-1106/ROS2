// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: mockBasicParameter.h
// Description: Mock class for the BasicParameter interface.

#ifndef mockBasicParameter_H
#define mockBasicParameter_H

#include <gmock/gmock.h>
#include "IBasicParameter.h"

namespace mock
{
class BasicParameter : public ::middleware::parameter::IBasic
{
public:
	virtual ~BasicParameter() override = default;
	MOCK_METHOD(::middleware::parameter::Identity, Identity, ());
	MOCK_METHOD(::middleware::parameter::Unit, Unit, ());
};
}  // namespace mock

#endif  // mockBasicParameter_H
