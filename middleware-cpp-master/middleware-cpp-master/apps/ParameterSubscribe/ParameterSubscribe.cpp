/******************************************************************************************************************
 Copyright 2024 Caterpillar Inc. All rights reserved.
-------------------------------------------------------------------------------------------------------------------
File name: ParameterSubscribe.cpp

Description:   Sample app for testing Gen7 Middleware - Publishing Simulator

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
#include "IPublishParameter.h"
#include "ParameterServiceFactory.h"

// Middleware typedefs
using Configuration = std::unique_ptr<middleware::parameter::IConfigBuilder>;

// Timing
#include <timediff_ns.h>
#define DEBUG_TIMING
#define SAMPLE_AVERAGE 50
string DebugLatencyChannel = "Debug.Latency";

class ParameterSubscribeImpl
{
public:
	ParameterSubscribeImpl() : m_TestReadSequential(false)
	{
	}

	// Read ParameterData stream
	static void ParameterSubscriberThread(ParameterSubscribeImpl* pAppServer, int* keepRunning)
	{
		thread::id ThreadID = this_thread::get_id();

		cout << "Begin ParameterSubscriberThread Thread [" << ThreadID << "] " << endl << std::flush;

		try
		{
			bool const ShowReadValues     = pAppServer->GetShowReadValues();
			bool const TestReadSequential = pAppServer->GetTestReadSequential();

			uint64_t LastDisplayValue = 1;  // init to known value for sequential check
			uint64_t SequentialMisses = 0;

#ifdef DEBUG_TIMING
			bool TossFirstTiming = true;

			// time sample receipt
			struct timespec monotime;
			clock_gettime(CLOCK_MONOTONIC, &monotime);

			// average ns values
			uint64_t RunningAverage[SAMPLE_AVERAGE];
			uint32_t WhichSample = 0;

			auto on_subscription = [ThreadID           = ThreadID,
			                        Show               = ShowReadValues,
			                        TestReadSequential = TestReadSequential,
			                        ShowReadValues     = ShowReadValues,
			                        &RunningAverage    = RunningAverage,
			                        &WhichSample       = WhichSample,
			                        &TossFirstTiming   = TossFirstTiming,
			                        &LastDisplayValue  = LastDisplayValue,
			                        &SequentialMisses  = SequentialMisses](auto const& id, auto const& data)
#else
			auto on_subscription = [ThreadID           = ThreadID,
			                        Show               = ShowReadValues,
			                        TestReadSequential = TestReadSequential,
			                        ShowReadValues     = ShowReadValues,
			                        &LastDisplayValue  = LastDisplayValue,
			                        &SequentialMisses  = SequentialMisses](auto const& id, auto const& data)
#endif
			{
#ifdef DEBUG_TIMING
				// only do timing on Latency Channel
				if(id == DebugLatencyChannel)
				{
					// time sample receipt
					struct timespec monotime;
					clock_gettime(CLOCK_MONOTONIC, &monotime);

					int64_t nstraveltime = 0;
					struct timespec monotxtime;
					memcpy(&monotxtime, data.value().binary_value().c_str(), sizeof(monotxtime));

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
						cout << "ParameterSubscriberThread [" << ThreadID << "] " << "Min " << Min << "|Max " << Max
						     << "|Avg " << Sum / SAMPLE_AVERAGE << " (micro)" << endl;
						if(TestReadSequential)
						{
							cout << "Thread [" << ThreadID << "] Sequential Mismatch Total Count|" << SequentialMisses
							     << endl;
						}
					}
				}
#endif
				// Show Read Values but skip Latency binary data
				if(ShowReadValues && (id != DebugLatencyChannel))
				{
					// printf("Sub RX|ID=%s|numeric=%f|%s\n", id.c_str(),
					// data.value().double_value(),Quality_Name(data.quality()).c_str());
					cout << "Sub RX|ID=" << id << "|uint64=" << data.value().uinteger64_value() << "|"
					     << Quality_Name(data.quality()) << endl;
				}

				if(TestReadSequential)
				{
					if(LastDisplayValue == 1)
					{
						LastDisplayValue = data.value().uinteger64_value() - 1;  // setup for 2nd point to work
					}
					else if(static_cast<int64_t>((LastDisplayValue + 1)) !=
					        static_cast<int64_t>(data.value().uinteger64_value()))
					{
						cout << "Thread [" << ThreadID << "] Sequential Mismatch Last|" << LastDisplayValue
						     << " Current|" << data.value().uinteger64_value() << endl;
						SequentialMisses++;
						cout << "Thread [" << ThreadID << "] Sequential Mismatch Total Count|" << SequentialMisses
						     << endl;
					}

					LastDisplayValue = data.value().uinteger64_value();
				}
			};
			auto on_disconnect = [WhichThread = ThreadID]
			{
				printf("[Disconnect Called]\n");
			};

			// Get Config Builder to setup test channels
			auto spConfigBuilder = middleware::parameter::IConfigBuilder::Create("DataLinkEngine");

			// Update config to include each channel as specified by command line arguments
			string id_string;
			char string_id_chars[10 + 1];

#ifdef DEBUG_TIMING
			string RobDoesntKnowHowCopyWorks = DebugLatencyChannel;
			spConfigBuilder->AddSubscribe(
			    std::move(RobDoesntKnowHowCopyWorks), {}, std::move(on_subscription), std::move(on_disconnect)
			);
#endif

			for(int i = 1; i <= pAppServer->GetStreamFilterRange(); i++)
			{
				snprintf(string_id_chars, sizeof(string_id_chars), "%i", i);
				id_string = string_id_chars;

				cout << "ParameterSubscriberThread [" << ThreadID << "] Subscribe to Parm " << id_string << endl
				     << std::flush;

				spConfigBuilder->AddSubscribe(
				    std::move(id_string), {}, std::move(on_subscription), std::move(on_disconnect)
				);
			}

			cout << "----------------------------------------------------------------" << endl;
			cout << "ParameterSubscriberThread [" << ThreadID << "] Total Parms " << pAppServer->GetStreamFilterRange()
			     << endl
			     << std::flush;
			cout << "----------------------------------------------------------------" << endl;

			// init server from config above
			auto spFactory = std::make_unique<::middleware::parameter::Factory>();
			auto spService = spFactory->CreateParameterService(std::move(spConfigBuilder));

			// Keep Thread Alive Until disconnect for sample app purposes
			// check every 1/4 second for shutdown
			while(*keepRunning == 1)
			{
				usleep(1 * 250 * 1000);
			}
		}
		catch(...)
		{
			cout << "Event Reader Exception Caught Thread [" << ThreadID << "] " << endl << std::flush;
		}

		cout << "End ParameterSubscriberThread Thread [" << ThreadID << "]" << endl << std::flush;
	}

	long GetStreamFilterRange() const
	{
		return m_StreamFilterRange;
	}

	long GetShowReadValues() const
	{
		return m_ShowReadValues;
	}

	bool GetTestReadSequential() const
	{
		return m_TestReadSequential;
	}

	void SetStreamFilterRange(long StreamFilterRange)
	{
		m_StreamFilterRange = StreamFilterRange;
	}

	void SetShowReadValues(bool ShowReadValues)
	{
		m_ShowReadValues = ShowReadValues;
	}

	void SetTestReadSequential(bool TestReadSequential)
	{
		m_TestReadSequential = TestReadSequential;
	}

private:
	long m_StreamFilterRange;
	bool m_ShowReadValues;
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

	long stream_filter_range  = -1;
	bool show_read_values     = false;
	bool test_read_sequential = false;

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
			else if(strncmp(argv[i - 1], "--streamfilterrange=", 20) == 0)
			{
				char rate[MAX_RATE_CHARS + 1];
				char* dontcare;
				strncpy(rate, (argv[i - 1]) + 20, MAX_RATE_CHARS);
				stream_filter_range = strtol(rate, &dontcare, 10);
				cout << "--streamfilterrange= specified " << stream_filter_range << endl;
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
				cout << "ParameterSubscribe\n--ip=<IPAddr:Port>\n--streamfilterrange=<filter UIDs from 1 this "
				        "value>\n--showreadvalues specifies show raw values\n--testreadsequential test each uint64 val "
				        "is +1 from previous (no missed message)\n--help Display This Usage Text"
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

	ParameterSubscribeImpl subscribeclient{};

	// setup stream max count for testing
	subscribeclient.SetStreamFilterRange(stream_filter_range);
	subscribeclient.SetShowReadValues(show_read_values);
	subscribeclient.SetTestReadSequential(test_read_sequential);

	if(subscribeclient.GetStreamFilterRange() > 0)
	{
		printf("StreamFilterRange 1-%li\n", subscribeclient.GetStreamFilterRange());
	}
	else
	{
		printf("StreamFilterRange Not Specified\n");
	}

	if(subscribeclient.GetShowReadValues())
	{
		printf("ShowReadValues TRUE\n");
	}
	else
	{
		printf("ShowReadValues FALSE\n");
	}

	// spin client thread to access middleware subscriber api
	thread ProducerUpdateStreamThread(ParameterSubscribeImpl::ParameterSubscriberThread, &subscribeclient, keepRunning);

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
