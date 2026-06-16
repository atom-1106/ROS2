// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ParametersComponentTest.h
// Description: Test Code for cate_middleware component MiddlewareParameterService

#ifndef ParametersComponentTest_H
#define ParametersComponentTest_H

#include <IBasicParameter.h>
#include <IConfigBuilder.h>
#include <IConfigTaker.h>
#include <IParameterService.h>
#include <IPublishParameter.h>
#include <IReadParameter.h>
#include <IWriteParameter.h>
#include <ParameterServiceDefines.h>
#include <ParameterServiceFactory.h>
#include <middleware_parameter.pb.h>

#include <iostream>
#include <string>

namespace test_package
{
class ParametersComponentTest
{
public:
	ParametersComponentTest(std::string const& parameter, std::string const& value)
	    : m_parameter{parameter}, m_value{value}

	{
	}
	virtual ~ParametersComponentTest() = default;
	bool Run()
	{
		std::cout << "[CHECK] Parameters Service...\n";

		auto spConfiguration = middleware::parameter::IConfigBuilder::Create("TestService");
		middleware::parameter::Data initial_value{};
		initial_value.mutable_value()->set_string_value(m_value);
		initial_value.set_quality(::cat::middleware::parameter::QUALITY_GOOD);
		spConfiguration->AddPublish(std::string{m_parameter}, {}, std::move(initial_value));

		auto spServiceInstance = ::middleware::parameter::Factory::CreateParameterService(std::move(spConfiguration));
		if(spServiceInstance)
		{
			std::cout << "[PASS] Parameters Service\n";
			return true;
		}
		else
		{
			std::cerr << "[FAILED] Parameters Service\n";
			return false;
		}
	}

private:
	std::string const m_parameter;
	std::string const m_value;
};

}  // namespace test_package

#endif  // ParametersComponentTest_H
