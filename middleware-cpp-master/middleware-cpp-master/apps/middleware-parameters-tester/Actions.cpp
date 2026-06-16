// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: Actions.cpp
// Description: Action handlers for middleware-parameters-tester.

#include "Actions.h"
#include <middleware_parameter.pb.h>
#include <boost/algorithm/hex.hpp>
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <string>
#include <thread>
#include "IConfigBuilder.h"
#include "IReadParameter.h"
#include "IWriteParameter.h"
#include "ParameterServiceDefines.h"
#include "ParameterServiceFactory.h"

namespace
{
/// @brief Converts a hex string (e.g. "0010230A18" or "0x0010230A18") to raw bytes.
std::string HexToBytes(std::string const& hex)
{
	std::string s = hex;
	if(s.size() >= 2 && (s.substr(0, 2) == "0x" || s.substr(0, 2) == "0X"))
	{
		s = s.substr(2);
	}
	return ::boost::algorithm::unhex(s);
}

/// @brief Constructs a protobuf Data message from the value flags in args.
///        Quality is set to QUALITY_GOOD.
::middleware::parameter::Data BuildData(::mpt::Args const& args)
{
	::middleware::parameter::Data data;
	data.set_quality(::cat::middleware::parameter::QUALITY_GOOD);

	if(args.has_double)
	{
		data.mutable_value()->set_double_value(args.double_val);
	}
	else if(args.has_string)
	{
		data.mutable_value()->set_string_value(args.string_val);
	}
	else if(args.has_binary)
	{
		data.mutable_value()->set_binary_value(HexToBytes(args.binary_val));
	}
	else if(args.has_boolean)
	{
		data.mutable_value()->set_boolean_value(args.boolean_val);
	}
	else if(args.has_uint64)
	{
		data.mutable_value()->set_uinteger64_value(args.uint64_val);
	}

	return data;
}

/// @brief Prints the identity and value held in a Data message to stdout.
void PrintData(std::string const& id, ::middleware::parameter::Data const& data)
{
	switch(data.value().selection_case())
	{
		case ::cat::middleware::parameter::Value::kDoubleValue:
			printf("id: [%s] value: [%f]\n", id.c_str(), data.value().double_value());
			break;
		case ::cat::middleware::parameter::Value::kStringValue:
			printf("id: [%s] value: [%s]\n", id.c_str(), data.value().string_value().c_str());
			break;
		case ::cat::middleware::parameter::Value::kBinaryValue:
		{
			printf("id: [%s] value: [0x", id.c_str());
			for(auto const c : data.value().binary_value())
			{
				printf("%02X", static_cast<uint8_t>(c));
			}
			printf("]\n");
			break;
		}
		case ::cat::middleware::parameter::Value::kBooleanValue:
			printf("id: [%s] value: [%s]\n", id.c_str(), data.value().boolean_value() ? "true" : "false");
			break;
		case ::cat::middleware::parameter::Value::kUinteger64Value:
			printf("id: [%s] value: [%lu]\n", id.c_str(), data.value().uinteger64_value());
			break;
		default: printf("id: [%s] value: [unknown type]\n", id.c_str()); break;
	}
}
}  // namespace

namespace mpt
{
void RunPub(Args const& args)
{
	auto const data  = BuildData(args);
	auto const& name = args.names[0];

	auto spConfig = ::middleware::parameter::IConfigBuilder::Create(k_ServerIdentity);
	spConfig->AddPublish(std::string(name), {}, ::middleware::parameter::Data{data});

	auto spService = ::middleware::parameter::Factory::CreateParameterService(std::move(spConfig));

	printf("[Publish] ");
	PrintData(name, data);

	std::this_thread::sleep_for(args.delay);
}

void RunSub(Args const& args)
{
	auto on_sub = [](::middleware::parameter::Identity const& id,
	                 ::middleware::parameter::Data const& data,
	                 ::middleware::parameter::Metadata const& /*metadata*/)
	{
		printf("[Subscription] ");
		PrintData(id, data);
	};

	auto spConfig = ::middleware::parameter::IConfigBuilder::Create(k_ServerIdentity);
	for(auto const& name : args.names)
	{
		auto on_disc = [name]
		{
			printf("[Subscription] disconnected: %s\n", name.c_str());
		};
		spConfig->AddSubscribe(std::string(name), on_sub, std::move(on_disc));
	}

	auto spService = ::middleware::parameter::Factory::CreateParameterService(std::move(spConfig));

	// Wait forever – the subscription callbacks drive all output.
	std::mutex mtx;
	std::condition_variable cv;
	std::unique_lock<std::mutex> lock{mtx};
	cv.wait(lock, [] { return false; });
}

void RunRead(Args const& args)
{
	auto spConfig = ::middleware::parameter::IConfigBuilder::Create(k_ServerIdentity);
	for(auto const& name : args.names)
	{
		spConfig->AddClientRead(std::string(name), {});
	}

	auto spService = ::middleware::parameter::Factory::CreateParameterService(std::move(spConfig));

	std::mutex mtx;
	std::condition_variable cv;
	int pending = static_cast<int>(args.names.size());

	auto on_complete = [&mtx, &cv, &pending](auto const& status, auto const& parameter)
	{
		std::string const status_str = (status.status == ::middleware::parameter::Status::SUCCESS) ? "SUCCESS" : "FAIL";
		printf("[Read Complete] status: [%s] ", status_str.c_str());
		PrintData(parameter.info().identity(), parameter.data());

		std::lock_guard<std::mutex> guard{mtx};
		--pending;
		cv.notify_one();
	};

	for(auto const& name : args.names)
	{
		spService->RetrieveReadInstance(name).Read(on_complete);
	}

	std::unique_lock<std::mutex> lock{mtx};
	cv.wait(lock, [&pending] { return pending == 0; });
}

void RunWrite(Args const& args)
{
	auto const data  = BuildData(args);
	auto const& name = args.names[0];

	auto spConfig = ::middleware::parameter::IConfigBuilder::Create(k_ServerIdentity);
	spConfig->AddClientWrite(std::string(name), {});

	auto spService = ::middleware::parameter::Factory::CreateParameterService(std::move(spConfig));

	std::mutex mtx;
	std::condition_variable cv;
	bool done = false;

	auto on_complete = [&mtx, &cv, &done](auto const& status, auto const& parameter)
	{
		std::string const status_str = (status.status == ::middleware::parameter::Status::SUCCESS) ? "SUCCESS" : "FAIL";
		printf("[Write Complete] status: [%s] ", status_str.c_str());
		PrintData(parameter.info().identity(), parameter.data());

		std::lock_guard<std::mutex> guard{mtx};
		done = true;
		cv.notify_one();
	};

	spService->RetrieveWriteInstance(name).Write(data, std::move(on_complete));

	std::unique_lock<std::mutex> lock{mtx};
	cv.wait(lock, [&done] { return done; });
}
}  // namespace mpt
