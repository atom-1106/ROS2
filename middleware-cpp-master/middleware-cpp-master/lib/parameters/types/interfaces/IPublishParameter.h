// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IPublishParameter.h
// Description: Interface API for external users.

#ifndef IPublishParameter_H
#define IPublishParameter_H

#include "IBasicParameter.h"
#include "ParameterServiceDefines.h"

namespace middleware
{
namespace parameter
{
class IPublish : public ::middleware::parameter::IBasic
{
public:
	virtual ~IPublish() override           = default;
	virtual void Publish(Data const& data) = 0;
};
}  // namespace parameter
}  // namespace middleware
#endif  // IPublishParameter_H
