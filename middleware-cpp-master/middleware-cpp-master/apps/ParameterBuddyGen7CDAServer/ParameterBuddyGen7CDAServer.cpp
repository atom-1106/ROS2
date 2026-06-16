/******************************************************************************************************************
 Copyright 2016 Caterpillar Inc. All rights reserved.
-------------------------------------------------------------------------------------------------------------------
File name: ParameterBuddyGen7CDA.cpp

Description:  Test app to test running write tests

******************************************************************************************************************/
#include <signal.h>
#include <sys/mman.h>
#include <iostream>
#include <memory>
#include <string>

// Middleware Includes
#include <middleware_parameter.pb.h>
#include "IConfigBuilder.h"
#include "IPublishParameter.h"
#include "IReadParameter.h"
#include "IWriteParameter.h"
#include "ParameterServiceFactory.h"

// Middleware typedefs
using Configuration = std::unique_ptr<middleware::parameter::IConfigBuilder>;

// grpc includes
#include <grpc++/grpc++.h>
#include <grpc++/security/server_credentials.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc/grpc.h>

// include our impl service classes
#include <CDAParameterConsumerService.h>
#include <CDAParameterProducerService.h>

using namespace std;
using namespace CDA::ParameterProducer;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

class ParameterBuddyGen7CDAThreadImpl
{
public:
	ParameterBuddyGen7CDAThreadImpl()
	{
	}

	// Read ParameterData stream
	static void BuddyGen7CDAThread(
	    ParameterBuddyGen7CDAThreadImpl* pAppServer,
	    CDAParameterProducerServiceImpl* pProducer,
	    CDAParameterConsumerServiceImpl* pConsumer,
	    int* keepRunning
	)
	{
		thread::id ThreadID = this_thread::get_id();

		cout << "Begin BuddyGen7CDAThread Thread [" << ThreadID << "] " << endl << std::flush;

		bool const ShowReadValues = pAppServer->GetShowReadValues();

		// Init CDA with the number of parms passed into buddy
		for(int i = 1; i <= pAppServer->GetStreamFilterRange(); i++)
		{
			ParameterInfo ParmInfo;
			ParmInfo.set_uid64(i);
			string Description = to_string(i);
			ParmInfo.set_parameterdescription(Description);
			ParmInfo.set_unitid(7);
			ParmInfo.set_unitsystem(ParameterInfo_UNIT_SYSTEM_METRIC);
			ParmInfo.set_datatype(ParameterInfo_DATA_TYPE_NUMERIC);
			ParmInfo.set_displayprecision(0);

			ParameterData ParmData;
			ParmData.set_uid64(i);
			ParmData.set_displayvalue(0.0);
			ParmData.set_dataquality(ParameterData_DATA_QUALITY_QUALITY_GOOD);

			CreateParameterChannelMessage ParmCreateChannel;
			*(ParmCreateChannel.mutable_channelparameterinfo())             = ParmInfo;
			*(ParmCreateChannel.mutable_channelparameterdatainitialvalue()) = ParmData;

			// Create Channel
			::grpc::ServerContext* pContext = nullptr;
			CreateParameterChannelReturn CreateReturnValue;
			pProducer->CDAParameterProducerServiceImpl::CreateParameterChannel(
			    pContext, &ParmCreateChannel, &CreateReturnValue
			);
			cout << "Created CDA Channel " << i << endl;
		}

		// init debug latency channel
		ParameterInfo ParmInfo;
		ParmInfo.set_uid64(0xFFFFFFFFFFFFFFFF);
		ParmInfo.set_parameterdescription("Debug.Latency");
		ParmInfo.set_unitid(7);
		ParmInfo.set_unitsystem(ParameterInfo_UNIT_SYSTEM_METRIC);
		ParmInfo.set_datatype(ParameterInfo_DATA_TYPE_BINARY);
		ParmInfo.set_displayprecision(0);

		ParameterData ParmData;
		ParmData.set_uid64(ParmInfo.uid64());
		ParmData.set_binaryvalue("");
		ParmData.set_dataquality(ParameterData_DATA_QUALITY_QUALITY_GOOD);

		CreateParameterChannelMessage ParmCreateChannel;
		*(ParmCreateChannel.mutable_channelparameterinfo())             = ParmInfo;
		*(ParmCreateChannel.mutable_channelparameterdatainitialvalue()) = ParmData;

		// Create Channel
		::grpc::ServerContext* pContext = nullptr;
		CreateParameterChannelReturn CreateReturnValue;
		pProducer->CDAParameterProducerServiceImpl::CreateParameterChannel(
		    pContext, &ParmCreateChannel, &CreateReturnValue
		);
		cout << "Created CDA Channel Debug.Latency" << ParmInfo.uid64() << endl;

		try
		{
			auto on_subscription = [ThreadID       = ThreadID,
			                        pAppServer     = pAppServer,
			                        pProducer      = pProducer,
			                        pConsumer      = pConsumer,
			                        ShowReadValues = ShowReadValues](auto const& id, auto const& data)
			{
				if(id == "Debug.Latency")
				{
					// republish as magic UID64 0xFFFFFFFFFFFFFFFF
					ParameterData ParmData;
					ParmData.set_uid64(0xFFFFFFFFFFFFFFFF);
					ParmData.set_binaryvalue(data.value().binary_value());
					ParmData.set_dataquality(ParameterData_DATA_QUALITY_QUALITY_GOOD);

					// publish the value
					::grpc::ServerContext* pContext = nullptr;
					::CDA::ParameterProducer::WriteParameterDataReturn response;
					pProducer->WriteParameterData(pContext, &ParmData, &response);
				}
				else
				{
					if(ShowReadValues)
					{
						printf(
						    "Gen7 CDA Server|ID=%s|uint64=%lu|%s\n",
						    id.c_str(),
						    data.value().uinteger64_value(),
						    Quality_Name(data.quality()).c_str()
						);
					}

					// convert from string identifier to actual uint64
					uint64_t UID64Numeric = stoull(id.c_str());

					ParameterData ParmData;
					ParmData.set_uid64(UID64Numeric);
					ParmData.set_displayvalue(data.value().uinteger64_value());
					ParmData.set_dataquality(ParameterData_DATA_QUALITY_QUALITY_GOOD);

					// publish the value
					::grpc::ServerContext* pContext = nullptr;
					::CDA::ParameterProducer::WriteParameterDataReturn response;
					pProducer->WriteParameterData(pContext, &ParmData, &response);
				}
			};
			auto on_disconnect = [WhichThread = ThreadID]
			{
				printf("[Disconnect Called]\n");
			};

			// Assign configuration
			auto spConfigBuilder = middleware::parameter::IConfigBuilder::Create("Display");

			// Update config to include each channel as specified by command line arguments
			string id_string;
			char string_id_chars[10 + 1];

			for(int i = 1; i <= pAppServer->GetStreamFilterRange(); i++)
			{
				snprintf(string_id_chars, sizeof(string_id_chars), "%i", i);
				id_string = string_id_chars;

				spConfigBuilder->AddSubscribe(
				    std::move(id_string), {}, std::move(on_subscription), std::move(on_disconnect)
				);

				cout << "BuddyGen7CDAThread [" << ThreadID << "] Subscribe to Parm " << id_string << endl << std::flush;
			}

			cout << "----------------------------------------------------------------" << endl;
			cout << "BuddyGen7CDAThread [" << ThreadID << "] Total Parms " << pAppServer->GetStreamFilterRange() << endl
			     << std::flush;
			cout << "----------------------------------------------------------------" << endl;

			// add subscription to Debug.Latency Channel
			spConfigBuilder->AddSubscribe("Debug.Latency", {}, std::move(on_subscription), std::move(on_disconnect));

			// init server outside while loop now that config has been setup
			auto spFactory = std::make_unique<::middleware::parameter::Factory>();
			auto spService = spFactory->CreateParameterService(std::move(spConfigBuilder));

			// Keep Thread Alive Until disconnect for sample
			// check every 1/4 second for shutdown
			while(*keepRunning == 1)
			{
				// printf("Awake %lis\n",awakeseconds++);
				usleep(1 * 250 * 1000);
			}
		}
		catch(...)
		{
			cout << "Event Reader Exception Caught Thread [" << ThreadID << "] " << endl << std::flush;
		}

		cout << "End BuddyGen7CDAThread Thread [" << ThreadID << "]" << endl << std::flush;
	}

	long GetStreamFilterRange() const
	{
		return m_StreamFilterRange;
	}

	void SetStreamFilterRange(long StreamFilterRange)
	{
		m_StreamFilterRange = StreamFilterRange;
	}

	bool GetShowReadValues() const
	{
		return m_ShowReadValues;
	}

	void SetShowReadValues(bool ShowReadValues)
	{
		m_ShowReadValues = ShowReadValues;
	}

private:
	long m_StreamFilterRange;
	bool m_ShowReadValues;
};

// Main
// ************
int* keepRunning;

void ExitProgram(int sig);

std::unique_ptr<Server>* pServerShutdown;

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

	// variable for server_address
	std::string server_address("127.0.0.1:50051");

	// how many parameters to mirror
	long stream_filter_range = -1;
	bool show_read_values    = false;

	if(argc > 1)
	{
		for(int i = 1; i <= argc; i++)
		{
			if(strncmp(argv[i - 1], "--ip=", 5) == 0)
			{
				char IPAddress[MAX_SERVER_ADDRESS + 1];

				strncpy(IPAddress, (argv[1]) + 5, MAX_SERVER_ADDRESS);
				server_address = IPAddress;
			}
			else if(strncmp(argv[i - 1], "--showreadvalues", 16) == 0)
			{
				show_read_values = true;
				cout << "--showreadvalues specified TRUE" << endl;
			}
			else if(strncmp(argv[i - 1], "--streamfilterrange=", 20) == 0)
			{
				char rate[MAX_RATE_CHARS + 1];
				char* dontcare;
				strncpy(rate, (argv[i - 1]) + 20, MAX_RATE_CHARS);
				stream_filter_range = strtol(rate, &dontcare, 10);
				cout << "--streamfilterrange= specified " << stream_filter_range << endl;
			}
			else if(strncmp(argv[i - 1], "--help", 6) == 0)
			{
				cout << "ParameterBuddyGen7CDA\n--ip=<IPAddr:Port>\n--streamfilterrange=<filter UIDs from 1 this "
				        "value>\n--showreadvalues specifies show raw values\n--help Display This Usage Text"
				     << endl;
			}
		}
	}

	cout << "IP " << server_address << endl;

	CDAParameterProducerServiceImpl Producer;
	CDAParameterConsumerServiceImpl Consumer(Producer);
	ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&Producer);
	builder.RegisterService(&Consumer);
	std::unique_ptr<Server> server(builder.BuildAndStart());

	// register server so it can be accessed from signal handler
	pServerShutdown = &server;

	std::cout << "After BuildAndStart Parameter Buddy Gen7 CDA server main" << std::endl;

	ParameterBuddyGen7CDAThreadImpl buddythread{};

	// setup stream max count for testing
	buddythread.SetStreamFilterRange(stream_filter_range);
	buddythread.SetShowReadValues(show_read_values);

	if(buddythread.GetStreamFilterRange() > 0)
	{
		printf("StreamFilterRange 1-%li\n", buddythread.GetStreamFilterRange());
	}
	else
	{
		printf("StreamFilterRange Not Specified\n");
	}

	if(buddythread.GetShowReadValues())
	{
		printf("ShowReadValues TRUE\n");
	}
	else
	{
		printf("ShowReadValues FALSE\n");
	}

	thread ProducerUpdateStreamThread(
	    ParameterBuddyGen7CDAThreadImpl::BuddyGen7CDAThread, &buddythread, &Producer, &Consumer, keepRunning
	);
	std::cout << "After BuddyGen7CDAThread Thread Spun Up and Off the Chain" << std::endl;

	// stay alive while thread running
	ProducerUpdateStreamThread.join();

	server->Wait();

	return 0;
}

void ExitProgram(int sig)
{
	if(*keepRunning == 1)
	{
		*keepRunning = 0;
		printf("Shutdown request\n");
	}

	if(pServerShutdown != 0)
	{
		(*pServerShutdown)->Shutdown();
	}
}
