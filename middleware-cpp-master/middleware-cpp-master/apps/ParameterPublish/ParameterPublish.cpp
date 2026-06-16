/******************************************************************************************************************
 Copyright 2024 Caterpillar Inc. All rights reserved.
-------------------------------------------------------------------------------------------------------------------
File name: ParameterPublish.cpp

Description:  Sample app for testing Gen7 Middleware - Publishing Simulator

******************************************************************************************************************/
// standard includes
#include <sys/time.h>
#include <unistd.h>

// standard c++ includes
#include <chrono>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <vector>
using namespace std;

// Middleware Includes
#include <middleware_parameter.pb.h>
#include "IConfigBuilder.h"
#include "IPublishParameter.h"
#include "ParameterServiceFactory.h"

// Middleware typedefs
using Configuration = std::unique_ptr<middleware::parameter::IConfigBuilder>;

// Timing
#include <timediff_ns.h>
#define DEBUG_TIMING
string DebugLatencyChannel = "Debug.Latency";

class ParameterPublishImpl
{
public:
	ParameterPublishImpl() : m_PublishOnce(false)
	{
	}

	// Thread for loading data
	static void ParameterPublishDataLoader(ParameterPublishImpl* pAppServer)
	{
		thread::id ThreadID = this_thread::get_id();

		cout << "Begin ParameterPublishDataLoader Thread [" << ThreadID << "]" << endl;

		long publish_frequency  = pAppServer->GetGroupPublishRate();
		long publish_parameters = pAppServer->GetGroupPublishParms();
		bool publish_once       = pAppServer->GetPublishOnce();
		uint64_t UniqueIntValue = 0;

		// set group write # parameters
		publish_frequency  = pAppServer->GetGroupPublishRate();
		publish_parameters = pAppServer->GetGroupPublishParms();

		cout << "ParameterPublishDataLoader: [" << ThreadID << "] Group Publish of " << publish_parameters << " @"
		     << publish_frequency << "ms" << endl;

		// never been any reason
		// -- or --
		// there is a time to die
		if(publish_frequency < 1)
		{
			cout << "End ParameterPublishDataLoader Thread Frequency (-1 = ABORT) [" << ThreadID << "] " << endl;
			return;
		}

		// Update config to include each channel
		char string_id_chars[10 + 1];
		string write_bytes;

		// assign value string
		write_bytes = to_string(UniqueIntValue);

		// Assign configuration
		auto spConfigBuilder = middleware::parameter::IConfigBuilder::Create("DataLinkEngine");

		// setup parameter channels for use in loop
		// do identifiers as 1 relative string
		middleware::parameter::Data publish_data{};

		for(int i = 1; i <= publish_parameters; i++)
		{
			// create numeric identifier string
			snprintf(string_id_chars, sizeof(string_id_chars), "%i", i);
			string id_string = string_id_chars;
			cout << "ParameterPublishDataLoader: Publishing ID=" << id_string << endl;

			spConfigBuilder->AddPublish(std::move(id_string), {}, std::move(publish_data));
		}
		cout << "ParameterPublishDataLoader: Created " << publish_parameters << " elements " << endl;

#ifdef DEBUG_TIMING
		// time sample receipt
		struct timespec monotime;
		clock_gettime(CLOCK_MONOTONIC, &monotime);
		string monotimestring;
		monotimestring.assign(reinterpret_cast<char const*>(&monotime), sizeof(monotime));
		write_bytes = monotimestring;
		publish_data.mutable_value()->set_binary_value(write_bytes);
		string RobDoesntKnowHowCopyWorks = DebugLatencyChannel;
		spConfigBuilder->AddPublish(std::move(RobDoesntKnowHowCopyWorks), {}, std::move(publish_data));
		cout << "ParameterPublishDataLoader: Created " << DebugLatencyChannel << endl;

#endif
		UniqueIntValue++;

		bool KeepWriting = true;

		// init server outside while loop now that config has been setup
		auto spFactory = std::make_unique<::middleware::parameter::Factory>();
		auto spService = spFactory->CreateParameterService(std::move(spConfigBuilder));

		cout << "ParameterPublishDataLoader: Publishing " << publish_parameters << " Parameters @" << publish_frequency
		     << "ms" << endl;
		string binaryvaluestring;

		// its all good - use same data for each write and just update binary value
		::middleware::parameter::Data data{};
		data.set_quality(::cat::middleware::parameter::QUALITY_GOOD);

		while(KeepWriting)
		{
			UniqueIntValue++;
			data.mutable_value()->set_uinteger64_value(UniqueIntValue);

			// update time in each channel (simulate publishing app)
			for(int i = 1; i <= publish_parameters; i++)
			{
				// create numeric identifier string
				snprintf(string_id_chars, sizeof(string_id_chars), "%i", i);
				string id_string = string_id_chars;

				spService->RetrievePublisherInstance(id_string).Publish(data);
			}

#ifdef DEBUG_TIMING
			middleware::parameter::Data publish_latency{};
			clock_gettime(CLOCK_MONOTONIC, &monotime);
			binaryvaluestring.clear();
			binaryvaluestring.insert(0, reinterpret_cast<char*>(&monotime), sizeof(monotime));
			publish_latency.mutable_value()->set_binary_value(binaryvaluestring);
			spService->RetrievePublisherInstance(DebugLatencyChannel).Publish(publish_latency);
#endif

			// sleep for publish rate
			if(publish_once)
			{
				KeepWriting = false;
			}
			else
			{
				this_thread::sleep_for(std::chrono::milliseconds(publish_frequency));
			}
		}

		cout << "End ParameterPublishDataLoader Thread [" << ThreadID << "]" << endl;
	}

	long GetGroupPublishRate() const
	{
		return m_PublishRate;
	}

	long GetGroupPublishParms() const
	{
		return m_PublishParms;
	}

	bool GetPublishOnce() const
	{
		return m_PublishOnce;
	}

	void SetGroupPublishParms(long GroupPublishParms)
	{
		m_PublishParms = GroupPublishParms;
	}

	void SetGroupPublishRate(long GroupPublishRate)
	{
		m_PublishRate = GroupPublishRate;
	}

	void SetPublishOnce(bool PublishOnce)
	{
		m_PublishOnce = PublishOnce;
	}

private:
	long m_PublishRate;
	long m_PublishParms;
	bool m_PublishOnce;
};

int main(int argc, char** argv)
{
	// Max String length = "192.168.100.123:50051"
	int const MAX_SERVER_ADDRESS = 21;
	int const MAX_RATE_CHARS     = 10;

	long publish_rate_ms = -1;
	long publish_parms   = -1;
	bool publish_once    = false;

	// variable for server_address
	std::string server_address("127.0.0.1:50075");

	if(argc > 1)
	{
		for(int i = 1; i <= argc; i++)
		{
			if(strncmp(argv[i - 1], "--ip=", 5) == 0)
			{
				char IPAddress[MAX_SERVER_ADDRESS + 1];

				strncpy(IPAddress, (argv[i - 1]) + 5, MAX_SERVER_ADDRESS);
				server_address = IPAddress;
			}
			else if(strncmp(argv[i - 1], "--publishrate=", 14) == 0)
			{
				char rate[MAX_RATE_CHARS + 1];
				char* dontcare;
				strncpy(rate, (argv[i - 1]) + 14, MAX_RATE_CHARS);
				publish_rate_ms = strtol(rate, &dontcare, 10);
				cout << "--publishrate= specified " << publish_rate_ms << endl;
			}
			else if(strncmp(argv[i - 1], "--publishparms=", 15) == 0)
			{
				char rate[MAX_RATE_CHARS + 1];
				char* dontcare;
				strncpy(rate, (argv[i - 1]) + 15, MAX_RATE_CHARS);
				publish_parms = strtol(rate, &dontcare, 10);
				cout << "--publishparms= specified " << publish_parms << endl;
			}
			else if(strncmp(argv[i - 1], "--publishonce", 13) == 0)
			{
				publish_once = true;
			}
			else if(strncmp(argv[i - 1], "--help", 6) == 0)
			{
				cout << "ParameterPublish\n--ip=<IPAddr:Port>\n--publishrate=<rate ms>\n--publishparms=<number of "
				        "parms>\n--publishonce\n--help Display This Usage Text"
				     << endl;
				return 0;
			}
		}
	}

	cout << "IP " << server_address << endl;

#ifdef DEBUG_TIMING
	cout << "Debug Timing TRUE" << endl;
#else
	cout << "Debug Timing FALSE" << endl;
#endif

	ParameterPublishImpl publishclient{};

	// setup and start sample data inserter thread - (even if rates are )
	publishclient.SetGroupPublishRate(publish_rate_ms);
	publishclient.SetGroupPublishParms(publish_parms);
	publishclient.SetPublishOnce(publish_once);

	printf(
	    "Publish Rate %ld | Publish Parms %ld\n",
	    publishclient.GetGroupPublishRate(),
	    publishclient.GetGroupPublishParms()
	);

	thread ProducerDataLoadThreadSingle(ParameterPublishImpl::ParameterPublishDataLoader, &publishclient);

	ProducerDataLoadThreadSingle.join();

	return 0;
}
