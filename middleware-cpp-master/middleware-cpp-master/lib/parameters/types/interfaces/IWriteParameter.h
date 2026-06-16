// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IWriteParameter.h
// Description: Interface API for external users.

#ifndef IWriteParameter_H
#define IWriteParameter_H

#include <functional>
#include "IBasicParameter.h"
#include "ParameterServiceDefines.h"

namespace middleware
{
namespace parameter
{
class IWrite : public ::middleware::parameter::IBasic
{
public:
	struct WriteStatus
	{
		Status status{Status::UNSPECIFIED};
	};
	using OnCompletion = std::function<void(WriteStatus const&, Parameter const&)>;

	virtual ~IWrite() override                                    = default;
	virtual void Write(Data const& data, OnCompletion&& callback) = 0;
};
}  // namespace parameter
}  // namespace middleware
#endif  // IWriteParameter_H
