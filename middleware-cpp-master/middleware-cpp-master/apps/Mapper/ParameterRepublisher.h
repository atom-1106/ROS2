// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ParameterRepublisher.h
// Description: Parameter Republisher class

#ifndef ParameterRepublisher_H
#define ParameterRepublisher_H

#include <memory>
#include <string>

namespace middleware
{
namespace parameter
{
class IConfigBuilder;
class IPublish;
class IService;
}  // namespace parameter
}  // namespace middleware

namespace cat::middleware::parameter
{
class Data;
}

class ParameterRepublisher final
{
public:
	ParameterRepublisher();
	~ParameterRepublisher();

	void Start();

private:
	std::unique_ptr<middleware::parameter::IConfigBuilder> CreateConfiguration();
	void OnParameterUpdated(std::string const& parameterName, cat::middleware::parameter::Data const& data);

	std::unique_ptr<middleware::parameter::IService> m_spParameterService;
	middleware::parameter::IPublish* m_pPublish{};
};

#endif  // ParameterRepublisher_H
