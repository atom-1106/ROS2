/******************************************************************************************************************
 Copyright 2024 Caterpillar Inc. All rights reserved.
-------------------------------------------------------------------------------------------------------------------
File name: ParameterReadClient.cpp

Description:   Sample app for testing Gen7 Middleware - Read Simulator

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

// Middleware typedefs
using Configuration = std::unique_ptr<middleware::parameter::IConfigBuilder>;

// Timing
#include <timediff_ns.h>
#define DEBUG_TIMING
#define SAMPLE_AVERAGE 50
char const DebugLatencyChannel[] = "Debug.Read.Latency";

class ParameterReadClientImpl
{
public:
	ParameterReadClientImpl() : m_ReadRate(1000), m_TestReadSequential(false)
	{
	}

	// Read ParameterData stream
	static void ParameterReadClientThread(ParameterReadClientImpl* pAppServer, int* keepRunning)
	{
		thread::id ThreadID = this_thread::get_id();

		cout << "Begin ParameterReadClientThread Thread [" << ThreadID << "] " << endl << std::flush;

		try
		{
			bool const ShowReadValues     = pAppServer->GetShowReadValues();
			bool const TestReadSequential = pAppServer->GetTestReadSequential();
			long const ReadFrequency      = pAppServer->GetReadRate();
			long const ReadParameters     = pAppServer->GetReadParms();
			bool const ReadOnce           = pAppServer->GetReadOnce();

			uint64_t LastDisplayValue    = 0;
			bool SetSequentialFirstValue = true;
			uint64_t SequentialMisses    = 0;

#ifdef DEBUG_TIMING
			bool TossFirstTiming = true;

			// time sample receipt
			struct timespec monotime;
			clock_gettime(CLOCK_MONOTONIC, &monotime);

			// average ns values
			uint64_t RunningAverage[SAMPLE_AVERAGE];
			uint32_t WhichSample = 0;

			auto on_read_client = [ThreadID                 = ThreadID,
			                       TestReadSequential       = TestReadSequential,
			                       ShowReadValues           = ShowReadValues,
			                       &RunningAverage          = RunningAverage,
			                       &WhichSample             = WhichSample,
			                       &TossFirstTiming         = TossFirstTiming,
			                       &SetSequentialFirstValue = SetSequentialFirstValue,
			                       &LastDisplayValue        = LastDisplayValue,
			                       &SequentialMisses = SequentialMisses](auto const& ReadStatus, auto const& Parameter)
#else
			auto on_read_client = [ThreadID                 = ThreadID,
			                       TestReadSequential       = TestReadSequential,
			                       ShowReadValues           = ShowReadValues,
			                       &SetSequentialFirstValue = SetSequentialFirstValue,
			                       &LastDisplayValue        = LastDisplayValue,
			                       &SequentialMisses = SequentialMisses](auto const& ReadStatus, auto const& Parameter)
#endif
			{
#ifdef DEBUG_TIMING
				// only do timing on Latency Channel
				if(Parameter.info().identity() == DebugLatencyChannel)
				{
					// time sample receipt
					struct timespec monotime;
					clock_gettime(CLOCK_MONOTONIC, &monotime);

					int64_t nstraveltime = 0;
					struct timespec monotxtime;
					memcpy(&monotxtime, Parameter.data().value().binary_value().c_str(), sizeof(monotxtime));

					if(monotime.tv_sec == monotxtime.tv_sec)
					{
						nstraveltime = monotime.tv_nsec - monotxtime.tv_nsec;
					}
					else
					{
						// filter out delays > 10 seconds as likely on different time basis
						if(abs(monotime.tv_sec - monotxtime.tv_sec) > 10)
						{
							// if we aren't on the same time base but print out status message
							if(++WhichSample == SAMPLE_AVERAGE)
							{
								// reset sample counter
								WhichSample = 0;
							}
						}
						else
						{
							// nano amount when rolled over =
							nstraveltime =
							    // (non second offset if > 1)
							    (((monotime.tv_sec - 1) - monotxtime.tv_sec) * 1000000000) +
							    // nano in current second + nano on original base second
							    monotime.tv_nsec + (1000000000 - monotxtime.tv_nsec);
						}
					}

					if(!TossFirstTiming)
					{
						RunningAverage[WhichSample++] = nstraveltime / 1000;
					}
					else
					{
						TossFirstTiming = false;
					}

					if(WhichSample == SAMPLE_AVERAGE)
					{
						// reset
						WhichSample = 0;

						// calculate average
						uint64_t Sum = 0;
						uint64_t Max = 0;
						uint64_t Min = std::numeric_limits<uint64_t>::max();

						for(int i = 0; i < SAMPLE_AVERAGE; i++)
						{
							Sum += RunningAverage[i];
							if(RunningAverage[i] > Max)
							{
								Max = RunningAverage[i];
							}
							if(RunningAverage[i] < Min)
							{
								if(RunningAverage[i] > 0)
								{
									Min = RunningAverage[i];
								}
							}
						}

						// print timing stats
						cout << "Read RX Stats|Min=" << Min << "|Max=" << Max << "|Avg=" << Sum / SAMPLE_AVERAGE
						     << " (micro)" << endl;
						if(TestReadSequential)
						{
							cout << "Read RX Sequential|Mismatch|" << SequentialMisses << endl;
						}
					}
				}
#endif
				// Show Read Values but skip Latency binary data
				if(ShowReadValues && (Parameter.info().identity() != DebugLatencyChannel))
				{
					cout << "Read RX|ID=" << Parameter.info().identity()
					     << "|uint64=" << Parameter.data().value().uinteger64_value() << "|"
					     << Quality_Name(Parameter.data().quality()) << endl;

					// Only Check Sequential on Parameter 1
					if(TestReadSequential && (Parameter.info().identity() == "1"))
					{
						if(SetSequentialFirstValue)
						{
							// First Mismatch is just setting up the sequence for next data update
							// Without counting it as a SequentialMiss
							SetSequentialFirstValue = false;
							LastDisplayValue        = Parameter.data().value().uinteger64_value();
							cout << "Read RX Sequential (init)|Misses=" << SequentialMisses
							     << "|Last=" << LastDisplayValue
							     << "|Current=" << Parameter.data().value().uinteger64_value() << endl;
						}
						else if((LastDisplayValue + 1) != Parameter.data().value().uinteger64_value())
						{
							SequentialMisses++;
							cout << "Read RX Sequential|Misses=" << SequentialMisses << "|Last=" << LastDisplayValue
							     << "|Current=" << Parameter.data().value().uinteger64_value() << endl;
						}
						LastDisplayValue = Parameter.data().value().uinteger64_value();
					}
				}
				else
				{
				}
			};

			// Get Config Builder to setup test channels
			auto spConfigBuilder = middleware::parameter::IConfigBuilder::Create("CoreTelematics");

			// Update config to include each channel as specified by command line arguments
			char string_id_chars[10 + 1];

			for(int i = 1; i <= ReadParameters; i++)
			{
				snprintf(string_id_chars, sizeof(string_id_chars), "%i", i);
				string id_string = string_id_chars;

				cout << "ParameterReadClientThread [" << ThreadID << "] Creating Read Client Parm " << id_string << endl
				     << std::flush;

				spConfigBuilder->AddClientRead(std::move(id_string), {});
			}
			cout << "----------------------------------------------------------------" << endl;
			cout << "ParameterReadClientThread [" << ThreadID << "] Total Parms " << ReadParameters << endl
			     << std::flush;
			cout << "----------------------------------------------------------------" << endl;

#ifdef DEBUG_TIMING
			cout << "ParameterReadClientThread [" << ThreadID << "] Creating Timing Channel " << DebugLatencyChannel
			     << endl
			     << std::flush;
			spConfigBuilder->AddClientRead(DebugLatencyChannel, {});
#endif
			// init server from config above
			auto spFactory = std::make_unique<::middleware::parameter::Factory>();
			auto spService = spFactory->CreateParameterService(std::move(spConfigBuilder));

			// Now that we have the AddClientRead for each parm - need to call active parm Read() function in loop
			cout << "Starting Read for " << ReadParameters << " Parms @" << ReadFrequency << "ms" << endl;

			// Keep Thread Alive Until disconnect for sample
			while(*keepRunning == 1)
			{
				for(int i = 1; i <= ReadParameters; i++)
				{
					// create numeric identifier string
					snprintf(string_id_chars, sizeof(string_id_chars), "%i", i);
					string id_string = string_id_chars;

					spService->RetrieveReadInstance(id_string).Read(std::move(on_read_client));
				}
#ifdef DEBUG_TIMING
				spService->RetrieveReadInstance(DebugLatencyChannel).Read(std::move(on_read_client));
#endif
				if(ReadOnce)
				{
					*keepRunning = 0;
					// ToDo: Block until 1st async callback is received
				}
				usleep(ReadFrequency * 1000);  // cmd line in milliseconds - need it in microseconds
			}
		}
		catch(...)
		{
			cout << "Event Reader Exception Caught Thread [" << ThreadID << "] " << endl << std::flush;
		}

		cout << "End ParameterReadClientThread Thread [" << ThreadID << "]" << endl << std::flush;
	}

	long GetReadParms() const
	{
		return m_ReadParms;
	}

	long GetShowReadValues() const
	{
		return m_ShowReadValues;
	}

	long GetReadOnce() const
	{
		return m_ReadOnce;
	}

	long GetReadRate() const
	{
		return m_ReadRate;
	}

	bool GetTestReadSequential() const
	{
		return m_TestReadSequential;
	}

	void SetReadParms(long ReadParms)
	{
		m_ReadParms = ReadParms;
	}

	void SetShowReadValues(bool ShowReadValues)
	{
		m_ShowReadValues = ShowReadValues;
	}

	void SetReadOnce(bool ReadOnce)
	{
		m_ReadOnce = ReadOnce;
	}

	void SetTestReadSequential(bool TestReadSequential)
	{
		m_TestReadSequential = TestReadSequential;
	}

	void SetReadRate(long ReadRate)
	{
		m_ReadRate = ReadRate;
	}

private:
	long m_ReadParms;
	bool m_ShowReadValues;
	bool m_ReadOnce;
	long m_ReadRate;
	bool m_TestReadSequential;
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

	long read_parms           = -1;
	bool show_read_values     = false;
	bool test_read_sequential = false;
	bool readonce             = false;
	long readrate_ms          = 1000;

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
			else if(strncmp(argv[i - 1], "--readonce", 10) == 0)
			{
				readonce = true;
				cout << "--readonce specified TRUE" << endl;
			}
			else if(strncmp(argv[i - 1], "--readrate=", 11) == 0)
			{
				char rate[MAX_RATE_CHARS + 1];
				char* dontcare;
				strncpy(rate, (argv[i - 1]) + 11, MAX_RATE_CHARS);
				readrate_ms = strtol(rate, &dontcare, 10);
				cout << "--readrate= specified " << readrate_ms << endl;
			}
			else if(strncmp(argv[i - 1], "--readparms=", 12) == 0)
			{
				char rate[MAX_RATE_CHARS + 1];
				char* dontcare;
				strncpy(rate, (argv[i - 1]) + 12, MAX_RATE_CHARS);
				read_parms = strtol(rate, &dontcare, 10);
				cout << "--readparms= specified " << read_parms << endl;
			}
			else if(strncmp(argv[i - 1], "--showreadvalues", 16) == 0)
			{
				show_read_values = true;
				cout << "--showreadvalues specified TRUE" << endl;
			}
			else if(strncmp(argv[i - 1], "--testreadsequential", 20) == 0)
			{
				test_read_sequential = true;
				cout << "--testreadsequential specified TRUE" << endl;
			}
			else if(strncmp(argv[i - 1], "--help", 6) == 0)
			{
				cout << "ParameterReadClient\n--ip=<IPAddr:Port>\n--readonce quit after one publish\n--readrate=<value "
				        "in ms (detault 1000)>\n--readparms=<filter UIDs from 1 this value>\n--showreadvalues "
				        "specifies show raw values\n--testreadsequential test each uint64 val is +1 from previous (no "
				        "missed message)\n--help Display This Usage Text"
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

	ParameterReadClientImpl middlewarereadclient{};

	// setup stream max count for testing
	middlewarereadclient.SetReadParms(read_parms);
	middlewarereadclient.SetShowReadValues(show_read_values);
	middlewarereadclient.SetTestReadSequential(test_read_sequential);
	middlewarereadclient.SetReadOnce(readonce);
	middlewarereadclient.SetReadRate(readrate_ms);

	// spin client thread to do blocked reads with grpc reader object
	thread ProducerUpdateStreamThread(
	    ParameterReadClientImpl::ParameterReadClientThread, &middlewarereadclient, keepRunning
	);

	// stay alive while thread running
	ProducerUpdateStreamThread.join();

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
