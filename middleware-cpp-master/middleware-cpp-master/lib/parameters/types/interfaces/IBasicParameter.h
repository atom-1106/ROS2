// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IBasicParameter.h
// Description: Interface API for external users.

#ifndef IBasicParameter_H
#define IBasicParameter_H

#include "ParameterServiceDefines.h"

namespace middleware
{
namespace parameter
{
class IBasic
{
public:
	virtual ~IBasic()                                    = default;
	virtual ::middleware::parameter::Identity Identity() = 0;
	virtual ::middleware::parameter::Unit Unit()         = 0;
};
}  // namespace parameter
}  // namespace middleware
#endif  // IBasicParameter_H
