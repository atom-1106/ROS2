// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IConfigRetriever.h
// Description: Interface for middleware to retrieve built config.

#pragma once

#include <memory>
#include "ParameterServiceDefines.h"

namespace middleware
{
namespace parameter
{
class ParameterService;

class IConfigTaker
{
public:
	virtual ~IConfigTaker() = default;

	virtual ParameterService&& Take() = 0;
};
}  // namespace parameter
}  // namespace middleware
