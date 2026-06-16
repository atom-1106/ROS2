// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IReadParameter.h
// Description: Interface API for external users.

#ifndef IReadParameter_H
#define IReadParameter_H

#include <functional>
#include "IBasicParameter.h"
#include "ParameterServiceDefines.h"

namespace middleware
{
namespace parameter
{
class IRead : public ::middleware::parameter::IBasic
{
public:
	struct ReadStatus
	{
		Status status{Status::UNSPECIFIED};
	};
	using OnRequest = std::function<void(ReadStatus const&, Parameter const&)>;

	virtual ~IRead() override               = default;
	virtual void Read(OnRequest&& callback) = 0;
};
}  // namespace parameter
}  // namespace middleware
#endif  // IReadParameter_H
