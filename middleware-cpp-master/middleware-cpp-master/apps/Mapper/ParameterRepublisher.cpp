// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ParameterRepublisher.cpp
// Description: Parameter Republisher class

#include "ParameterRepublisher.h"
#include <middleware_parameter.pb.h>
#include <cassert>
#include <iostream>
#include "IConfigBuilder.h"
#include "IPublishParameter.h"
#include "IReadParameter.h"
#include "ParameterServiceFactory.h"

namespace
{
void OnPublisherDisconnected(char const* pId)
{
	std::cout << "Publisher disconnected for parameter: " << pId << std::endl;
}

constexpr char n_pSubscriberIdentity[]    = "Mapper";
constexpr char const* n_inputParameters[] = {
    "A",                     // sent by tests
    "machine.fuel_level_1",  // sent by Jaws
};
constexpr char n_outputParameter[] = "B";

constexpr uint8_t n_logLevel = 4;
}  // namespace

ParameterRepublisher::ParameterRepublisher()  = default;
ParameterRepublisher::~ParameterRepublisher() = default;

void ParameterRepublisher::Start()
{
	middleware::parameter::Factory::SetLogLevel(std::cout, n_logLevel);

	auto spConfigBuilder = CreateConfiguration();
	m_spParameterService = middleware::parameter::Factory::CreateParameterService(std::move(spConfigBuilder));
	m_pPublish           = &m_spParameterService->RetrievePublisherInstance(n_outputParameter);

	assert(m_spParameterService);
	assert(m_pPublish);
	std::cout << "Middleware parameter service created successfully" << std::endl;
}

void ParameterRepublisher::OnParameterUpdated(std::string const& id, cat::middleware::parameter::Data const& data)
{
	std::cout << "Republishing " << id << " as " << n_outputParameter << '\n';  // Don't flush yet (for timings)
	m_pPublish->Publish(data);
	std::cout.flush();
}

std::unique_ptr<middleware::parameter::IConfigBuilder> ParameterRepublisher::CreateConfiguration()
{
	auto spConfigBuilder = middleware::parameter::IConfigBuilder::Create(n_pSubscriberIdentity);

	for(char const* pId : n_inputParameters)
	{
		std::cout << "Subscribing to: " << pId << std::endl;

		auto onPublish = [this](std::string const& id, middleware::parameter::Data const& data)
		{
			this->OnParameterUpdated(id, data);
		};

		auto onDisconnect = [pId]()
		{
			OnPublisherDisconnected(pId);
		};

		spConfigBuilder->AddSubscribe(pId, {}, std::move(onPublish), std::move(onDisconnect));
	}

	spConfigBuilder->AddPublish(n_outputParameter, {}, {});

	return spConfigBuilder;
}
