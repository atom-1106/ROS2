/******************************************************************************************************************
 Copyright 2024 Caterpillar Inc. All rights reserved.
-------------------------------------------------------------------------------------------------------------------
File name: ParameterReadServer.cpp

Description:  Sample app for testing Gen7 Middleware - Parameter Active Read Simulator

******************************************************************************************************************/
// standard includes
#include <signal.h>
#include <sys/mman.h>
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
#include "IReadParameter.h"
#include "ParameterServiceFactory.h"

// Timing
#include <timediff_ns.h>
#define DEBUG_TIMING
char const DebugLatencyChannel[] = "Debug.Read.Latency";

class ParameterReadServerImpl
{
public:
	ParameterReadServerImpl()
	{
	}

	// Thread Launched to setup read server handler
	static void ParameterReadServerThread(ParameterReadServerImpl* pAppServer, bool single, int* keepRunning)
	{
		thread::id ThreadID = this_thread::get_id();

		cout << "Begin ParameterReadServerImpl [" << ThreadID << "]" << endl;

		// Update config to include each channel
		char string_id_chars[10 + 1];
		string write_bytes;
		uint64_t UniqueIntValue = 0;

		// assign id string
		snprintf(string_id_chars, sizeof(string_id_chars), "%i", 1);

		auto on_read_server = [&UniqueIntValue](auto, auto const& id)
		{
			::middleware::parameter::Data data;

			// its all good - use same data for each write and just update binary value
			data.set_quality(::cat::middleware::parameter::QUALITY_GOOD);

			// update monotonic time each time we get an active read
			// or update numeric value only when id is 1

			// update latency value when read
			if(id == DebugLatencyChannel)
			{
				// time sample receipt
				struct timespec monotime;
				clock_gettime(CLOCK_MONOTONIC, &monotime);
				string monotimestring;
				monotimestring.assign(reinterpret_cast<char const*>(&monotime), sizeof(monotime));

				// write first item
				data.mutable_value()->set_binary_value(monotimestring);
			}
			else
			{
				// increment only when item 1 is read - all others get same value
				if(id == "1")
				{
					UniqueIntValue++;
				}
				data.mutable_value()->set_uinteger64_value(UniqueIntValue);
			}
			return data;
		};

		// Get Config Builder to setup test channels
		auto spConfigBuilder = middleware::parameter::IConfigBuilder::Create("DataLinkEngine");

		// setup parameter channels for use in loop
		// do UID64s as 1 relative
		// in single loop add parameters and assign lambda for selected parameters
		for(int i = 1; i <= pAppServer->GetReadParameterCount(); i++)
		{
			// create numeric identifier string
			snprintf(string_id_chars, sizeof(string_id_chars), "%i", i);
			string id_string = string_id_chars;

			cout << "ParameterReadServerImpl [" << ThreadID << "] Server Read " << id_string << endl << std::flush;

			spConfigBuilder->AddServerRead(std::move(id_string), std::move(on_read_server));
		}

#ifdef DEBUG_TIMING
		spConfigBuilder->AddServerRead(DebugLatencyChannel, std::move(on_read_server));
#endif
		// init server outside while loop now that config has been setup
		auto spFactory = std::make_unique<::middleware::parameter::Factory>();
		auto spService = spFactory->CreateParameterService(std::move(spConfigBuilder));

		cout << "ParameterReadServerImpl Setup for " << pAppServer->GetReadParameterCount() << " parms" << endl;

		while(*keepRunning == 1)
		{
			usleep(1 * 250 * 1000);
		}

		cout << "End Parameter Read Server Thread [" << ThreadID << "]" << endl;
	}

	int32_t GetReadParameterCount() const
	{
		return m_ReadParameterCount;
	}

	void SetReadParameterCount(int32_t ReadParameterCount)
	{
		m_ReadParameterCount = ReadParameterCount;
	}

	void SetPublishOnce()
	{
		m_PublishOnce = true;
	}

	bool GetPublishOnceStatus()
	{
		return m_PublishOnce;
	}

private:
	int32_t m_ReadParameterCount;
	bool m_PublishOnce = false;
};

int* keepRunning;

void ExitProgram(int sig);

int main(int argc, char** argv)
{
	struct sigaction shutdownSigaction, pipeSigaction;

	/* Set up flag to handle SIGTERM/SIGINT shutdowns */
	keepRunning  = (int*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
	*keepRunning = 1;

	/* Set up signal action for SIGTERM/SIGINT */
	memset(&shutdownSigaction, 0, sizeof(shutdownSigaction));
	shutdownSigaction.sa_handler = ExitProgram;
	shutdownSigaction.sa_flags   = SA_RESTART;
	if(sigaction(SIGINT, &shutdownSigaction, NULL) != 0)
	{
		printf("Error registering for SIGINT action, errno = %d", errno);
		exit(1);
	}
	if(sigaction(SIGTERM, &shutdownSigaction, NULL) != 0)
	{
		printf("Error registering for SIGTERM action, errno = %d", errno);
		exit(1);
	}

	/* Set up signal action for SIGPIPE */
	memset(&pipeSigaction, 0, sizeof(pipeSigaction));
	pipeSigaction.sa_handler = SIG_IGN;
	pipeSigaction.sa_flags   = SA_RESTART;
	if(sigaction(SIGPIPE, &pipeSigaction, NULL) != 0)
	{
		printf("Error registering for SIGPIPE action, errno = %d", errno);
	}

	// Max String length = "192.168.100.123:50051"
	int const MAX_SERVER_ADDRESS = 21;
	int const MAX_RATE_CHARS     = 10;

	long server_read_parms = -1;
	bool bPublishOnce      = false;

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

			else if(strncmp(argv[i - 1], "--serverreadparms=", 18) == 0)
			{
				char rate[MAX_RATE_CHARS + 1];
				char* dontcare;
				strncpy(rate, (argv[i - 1]) + 18, MAX_RATE_CHARS);
				server_read_parms = strtol(rate, &dontcare, 10);
				cout << "--serverreadparms= specified " << server_read_parms << endl;
			}
			else if(strncmp(argv[i - 1], "--publishonce", 13) == 0)
			{
				bPublishOnce = true;
			}
			else if(strncmp(argv[i - 1], "--help", 6) == 0)
			{
				cout << "ParameterReadServer\n--ip=<IPAddr:Port>\n--serverreadparms=<number of "
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

	if(server_read_parms == 0)
	{
		cout << "Why don't you just tell me how many parameters you want to see? --serverreadparms" << endl;
		return -1;
	}

	ParameterReadServerImpl middlewarereadserver{};
	middlewarereadserver.SetReadParameterCount(server_read_parms);

	thread ProducerDataLoadThreadSingle(
	    ParameterReadServerImpl::ParameterReadServerThread, &middlewarereadserver, bPublishOnce, keepRunning
	);

	ProducerDataLoadThreadSingle.join();

	return 0;
}

void ExitProgram(int sig)
{
	if(*keepRunning == 1)
	{
		*keepRunning = 0;
		printf("Shutdown request\n");
	}
}
