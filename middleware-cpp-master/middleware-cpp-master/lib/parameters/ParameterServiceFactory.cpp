// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ParameterServiceFactory.cpp
// Description: Interface API for external users.

#include "ParameterServiceFactory.h"
#include "IConfigTaker.h"
#include "ParameterService.h"
#include "SystemUtilities.h"

namespace middleware
{
namespace parameter
{
Factory::ParameterServicePtr Factory::CreateParameterService(std::unique_ptr<IConfigTaker>&& spConfig)
{
	return std::make_unique<::middleware::parameter::Service>(spConfig->Take());
}
void Factory::SetLogLevel(std::ostream& out, uint8_t const level)
{
	::middleware::log::Reset();
	switch(level)
	{
		case 1:
			::middleware::log::Initialize(out, ::middleware::log::LogLevel::LEVEL_ONE);
			::middleware::log::Info("[MiddlewareLogger] LEVEL_ONE{ERROR}. Errors throw exceptions.");
			break;
		case 2:
			::middleware::log::Initialize(out, ::middleware::log::LogLevel::LEVEL_TWO);
			::middleware::log::Info("[MiddlewareLogger] LEVEL_TWO{ERROR|WARNINGS}. Errors throw exceptions.");
			break;
		case 3:
			::middleware::log::Initialize(out, ::middleware::log::LogLevel::LEVEL_THREE);
			::middleware::log::Info("[MiddlewareLogger] LEVEL_THREE{ERROR|WARNINGS|INFO}. Errors throw exceptions.");
			break;
		case 4:
			::middleware::log::Initialize(out, ::middleware::log::LogLevel::LEVEL_FOUR);
			::middleware::log::Info(
			    "[MiddlewareLogger] LEVEL_FOUR{ERROR|WARNINGS|INFO|DEBUG}. Errors throw exceptions."
			);
			break;
		default: out << "Log level [" << level << "] is not supported. Choices: [1,2,3,4]\n"; break;
	}
}
}  // namespace parameter
}  // namespace middleware
