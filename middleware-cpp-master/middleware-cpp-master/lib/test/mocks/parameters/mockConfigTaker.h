// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: mockConfigTaker.h
// Description: Mock class for the ConfigTaker interface.

#ifndef mockConfigTaker_H
#define mockConfigTaker_H

#include <gmock/gmock.h>
#include "IConfigTaker.h"

namespace mock
{
class ConfigTaker : public ::middleware::parameter::IConfigTaker
{
public:
	virtual ~ConfigTaker() override = default;
	MOCK_METHOD(::middleware::parameter::ParameterService&&, Take, ());
};
}  // namespace mock

#endif  // mockConfigTaker_H
