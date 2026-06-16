// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: main.cpp
// Description: Basic fake data link engine application tool for testing.

#include "IConfigBuilder.h"
#include "IPublishParameter.h"
#include "IReadParameter.h"
#include "IWriteParameter.h"
#include "ParameterServiceFactory.h"

#include <middleware_parameter.pb.h>
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstdarg>
#include <future>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;
using Configuration = std::unique_ptr<middleware::parameter::IConfigBuilder>;
std::string GetSystemTime()
{
	static constexpr char FORMAT[] = "%Y-%m-%d %X";
	using CLOCK                    = std::chrono::system_clock;
	auto in_time_t                 = CLOCK::to_time_t(CLOCK::now());
	std::stringstream ss;
	struct tm buf;
	gmtime_r(&in_time_t, &buf);
	ss << std::put_time(&buf, FORMAT);
	return ss.str();
}

void OnComplete(
    ::middleware::parameter::IRead::ReadStatus const& status,
    ::middleware::parameter::Parameter const& parameter
)
{
	std::string status_str = (status.status == ::middleware::parameter::SUCCESS) ? "SUCCESS" : "FAIL";
	printf(
	    "[%s] [ON_COMPLETE] id: [%s] status: [%s] value: [%lu]\n",
	    GetSystemTime().c_str(),
	    parameter.info().identity().c_str(),
	    status_str.c_str(),
	    parameter.data().value().uinteger64_value()
	);
}
void OnSubscription(::std::string const& identity, ::middleware::parameter::Data const& data)
{
	printf(
	    "[%s] [SUBSCRIPTION] id: [%s] quality: [%s] value: [%lu]\n",
	    GetSystemTime().c_str(),
	    identity.c_str(),
	    (data.quality() == ::cat::middleware::parameter::QUALITY_GOOD ? "GOOD" : "BAD"),
	    data.value().uinteger64_value()
	);
}
::cat::middleware::parameter::Data OnRead(::std::string const& identity)
{
	uint64_t value = UINT32_MAX;
	::cat::middleware::parameter::Data data{};
	data.mutable_value()->set_uinteger64_value(value);
	data.set_quality(::cat::middleware::parameter::QUALITY_GOOD);

	printf(
	    "[%s] [ON_READ] id: [%s] value: [%lu]\n",
	    GetSystemTime().c_str(),
	    identity.c_str(),
	    data.value().uinteger64_value()
	);
	return data;
}
void OnWrite(::std::string const& identity, ::middleware::parameter::Data const& data)
{
	printf(
	    "[%s] [ON_WRITE] id: [%s] value: [%lu]\n",
	    GetSystemTime().c_str(),
	    identity.c_str(),
	    data.value().uinteger64_value()
	);
}

int main(int argc, char** argv)
{
	try
	{
		std::string self_name = argv[1];
		uint32_t count        = 0;
		std::stringstream ss{argv[2]};
		ss >> count;

		if(count <= 1000u)
		{

			auto spConfigBuilder = middleware::parameter::IConfigBuilder::Create(std::string{self_name});

			// Add Publishers
			for(size_t i = 1; i <= count; i++)
			{
				::middleware::parameter::Data initial_value{};
				initial_value.mutable_value()->set_uinteger64_value(0);
				initial_value.set_quality(::cat::middleware::parameter::QUALITY_GOOD);

				std::string pub_name = self_name + "::Parameter" + std::to_string(i);
				printf("pub_name: %s\n", pub_name.c_str());

				spConfigBuilder->AddPublish(std::move(pub_name), {}, std::move(initial_value));
			}

			for(int i = 3; i < argc; i++)
			{
				std::string target_name = argv[i];
				for(size_t i = 1; i <= count; i++)
				{
					std::string sub_name = target_name + "::Parameter" + std::to_string(i);
					printf("sub_name: %s\n", sub_name.c_str());

					spConfigBuilder->AddSubscribe(
					    std::move(sub_name),
					    {},
					    [](auto const& id, auto const& data) { OnSubscription(id, data); },
					    []() {}
					);
				}
			}

			auto spService = ::middleware::parameter::Factory::CreateParameterService(std::move(spConfigBuilder));

			uint32_t current_value = 1;
			::middleware::parameter::Data next_value{};
			next_value.set_quality(::cat::middleware::parameter::QUALITY_GOOD);
			while(true)
			{
				next_value.mutable_value()->set_uinteger64_value(current_value++);
				for(size_t i = 1; i <= count; i++)
				{
					std::string parameter_name = self_name + "::Parameter" + std::to_string(i);
					auto& rParameter           = spService->RetrievePublisherInstance(parameter_name);
					rParameter.Publish(next_value);
				}
				if(current_value > UINT8_MAX)
				{
					current_value = 0;
				}
				std::this_thread::sleep_for(100ms);
			}
		}
		else
		{
			printf("Count must be <= 1000\n");
		}
	}
	catch(std::exception const& e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}
	return 0;
}
