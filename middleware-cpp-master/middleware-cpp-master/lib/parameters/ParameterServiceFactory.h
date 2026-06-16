// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ParameterServiceFactory.h
// Description: Interface API for external users.

#ifndef ParameterServiceFactory_H
#define ParameterServiceFactory_H

#include <memory>
#include <ostream>
#include "IParameterService.h"

namespace middleware
{
namespace parameter
{
class IConfigTaker;

class Factory
{
public:
	using ParameterServicePtr = std::unique_ptr<::middleware::parameter::IService>;
	static ParameterServicePtr CreateParameterService(std::unique_ptr<IConfigTaker>&& spConfig);
	static void SetLogLevel(std::ostream& out, uint8_t const level);
};

}  // namespace parameter
}  // namespace middleware
#endif  // ParameterServiceFactory_H
