/******************************************************************************************************************
 Copyright 2016 Caterpillar Inc. All rights reserved.
-------------------------------------------------------------------------------------------------------------------
File name: CDAParamsMirrorClientReader.cpp

Description:  Sample CDA for Consumer Stream Testing

******************************************************************************************************************/
// standard includes
#include <sys/time.h>
#include <unistd.h>

// standard c++ includes
#include <chrono>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <thread>
using namespace std;

// service definitions
#include <CDAParameterProducerService.h>

// probobuf includes
#include <google/protobuf/descriptor.h>
#include <grpc++/grpc++.h>

#include <timediff_ns.h>

// include our generated files and setup our namespace
#include <CDAParameterConsumer.grpc.pb.h>
#include <CDAParameterConsumer.pb.h>
using namespace CDA::ParameterConsumer;

#include <CDAParameterProducerService.h>
#include <ParamConsumerServerService.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

typedef std::unique_ptr<::grpc::ClientReader<::CDA::ParameterConsumer::ParameterData>> ParameterDataStreamReader;
typedef std::unique_ptr<::grpc::ClientReader<::CDA::ParameterConsumer::ParameterDataGroupMessage>> PeriodicStreamReader;

// Timing
#define DEBUG_TIMING
#define SAMPLE_AVERAGE 50

class CDAMirrorClientReaderImpl
{
public:
	CDAMirrorClientReaderImpl(std::shared_ptr<Channel> channel)
	    : ParameterConsumer_stub_(ParameterConsumer::NewStub(channel))
	{
		m_TestReadSequential = false;
	}

	void GetParameterDataStream(
	    ClientContext& context,
	    ParameterDataStreamFilterGroupMessage& Filter,
	    ParameterDataStreamReader& TransactionReader
	)
	{
		// Get Reader
		TransactionReader = ParameterConsumer_stub_->GetParameterDataStream(&context, Filter);
	}

	void GetPeriodicDataStream(
	    ClientContext& context,
	    PeriodicDataRequest& DataRequest,
	    PeriodicStreamReader& TransactionReader
	)
	{
		// Get Reader
		TransactionReader = ParameterConsumer_stub_->GetPeriodicParameterDataStream(&context, DataRequest);
	}

	// Read ParameterData stream
	static void MirrorReadUpdateStream(CDAMirrorClientReaderImpl* pClientApp)
	{
		thread::id ThreadID = this_thread::get_id();

		cout << "Begin MirrorReadUpdateStream Thread [" << ThreadID << "] " << endl << std::flush;

		try
		{
			// get reader interface outside loop
			ParameterDataStreamReader ThreadEventReader;

			// Context for the client. It could be used to convey extra information to
			// the server and/or tweak certain RPC behaviors.
			ClientContext context;

			// setup filter object for testing
			ParameterDataStreamFilterGroupMessage Filter;
			for(int i = 1; i <= pClientApp->GetStreamFilterRange(); i++)
			{
				Filter.add_parameterdatastreamfiltergroup()->set_uid64(i);
				cout << "MirrorReadUpdateStream Thread [" << ThreadID << "] Setting Filter UID64 " << i << endl
				     << std::flush;
			}
			cout << "----------------------------------------------------------------" << endl;
			cout << "MirrorReadUpdateStream Thread [" << ThreadID << "] Total Filters "
			     << Filter.parameterdatastreamfiltergroup_size() << endl
			     << std::flush;
			cout << "----------------------------------------------------------------" << endl;

			// Read Debug.Latency Channel
			Filter.add_parameterdatastreamfiltergroup()->set_uid64(0xFFFFFFFFFFFFFFFF);
			cout << "MirrorReadUpdateStream Thread [" << ThreadID
			     << "] Adding Debug.Latency Filter UID64 0xFFFFFFFFFFFFFFFF" << endl
			     << std::flush;

			pClientApp->GetParameterDataStream(context, Filter, ThreadEventReader);

			uint32_t SampleCount           = 0;
			uint32_t const SampleCountQuit = pClientApp->GetStreamClientMaxUpdates();
			bool const ShowReadValues      = pClientApp->GetShowReadValues();
			bool const TestReadSequential  = pClientApp->GetTestReadSequential();

#ifdef DEBUG_TIMING
			bool TossFirstTiming = true;

			// time sample receipt
			struct timespec monotime;
			clock_gettime(CLOCK_MONOTONIC, &monotime);

			// average ns values
			uint64_t RunningAverage[SAMPLE_AVERAGE];
			uint32_t WhichSample = 0;
#endif

			// print column headers if in show value mode
			if(ShowReadValues)
			{
				cout << "Count | UID64 | String | Float | Binary" << endl;
			}

			// blocked read
			// until stream closed
			ParameterData PublisherStreamedData;
			double LastDisplayValue   = -1.0;
			uint64_t SequentialMisses = 0;
			while(ThreadEventReader->Read(&PublisherStreamedData))
			{
				SampleCount++;

				if(ShowReadValues && (PublisherStreamedData.uid64() != 0xFFFFFFFFFFFFFFFF))
				{
					cout << "Gen7 CDA Client|ID=" << PublisherStreamedData.uid64()
					     << "|uint64=" << PublisherStreamedData.displayvalue() << endl;
				}

				if(TestReadSequential)
				{
					if(LastDisplayValue == -1)
					{
						LastDisplayValue = PublisherStreamedData.displayvalue() - 1;  // setup for 2nd point to work
					}
					else if(static_cast<int64_t>((LastDisplayValue + 1)) !=
					        static_cast<int64_t>(PublisherStreamedData.displayvalue()))
					{
						cout << "Thread [" << ThreadID << "] Sequential Mismatch Last|" << LastDisplayValue
						     << " Current|" << PublisherStreamedData.displayvalue() << endl;
						SequentialMisses++;
						cout << "Thread [" << ThreadID << "] Sequential Mismatch Total Count|" << SequentialMisses
						     << endl;
					}

					LastDisplayValue = PublisherStreamedData.displayvalue();
				}

				if((SampleCountQuit > 0) && (SampleCount >= SampleCountQuit))
				{
					cout << "MirrorReadUpdateStream: Exit Max Reads Exceeded " << SampleCountQuit << endl;
					break;  // out of while
				}

#ifdef DEBUG_TIMING
				if(PublisherStreamedData.uid64() == 0xFFFFFFFFFFFFFFFF)
				{
					// time sample receipt
					clock_gettime(CLOCK_MONOTONIC, &monotime);

					int64_t nstraveltime = 0;
					struct timespec monotxtime;
					memcpy(&monotxtime, PublisherStreamedData.binaryvalue().c_str(), sizeof(monotxtime));

					if(monotime.tv_sec == monotxtime.tv_sec)
					{
						nstraveltime = monotime.tv_nsec - monotxtime.tv_nsec;
					}
					else
					{
						// filter out delays > 10 seconds as likely on different time basis
						if(abs(monotime.tv_sec - monotxtime.tv_sec) > 10)
						{
							cout << "Message Sec=" << monotxtime.tv_sec << "|Now Sec=" << monotime.tv_sec << endl;

							// is this on two different ECUs - can't easily measure nanosecond timing
							// skip all this as we aren't on the same time base but print out status message
							// with number of data element received when % SAMPLE_AVERAGE
							if(++WhichSample == SAMPLE_AVERAGE)
							{
								// reset sample counter
								WhichSample = 0;
								cout << "Thread [" << ThreadID << "] " << "Samples Received " << SAMPLE_AVERAGE
								     << " (different timebase)" << endl;
							}

							// go back to blocked read
							continue;
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
						cout << "Thread [" << ThreadID << "] " << "Min " << Min << "|Max " << Max << "|Avg "
						     << Sum / SAMPLE_AVERAGE << " (micro)" << endl;
						if(TestReadSequential)
						{
							cout << "Thread [" << ThreadID << "] Sequential Mismatch Total Count|" << SequentialMisses
							     << endl;
						}
					}
				}
#endif
			}  // end while
		}
		catch(...)
		{
			cout << "Event Reader Exception Caught Thread [" << ThreadID << "] " << endl << std::flush;
		}

		cout << "End MirrorReadUpdateStream Thread [" << ThreadID << "]" << endl << std::flush;
	}

	// Read ParameterData stream
	static void MirrorReadPeriodicStream(CDAMirrorClientReaderImpl* pClientApp)
	{
		thread::id ThreadID = this_thread::get_id();

		cout << "Begin MirrorReadPeriodicStream Thread [" << ThreadID << "] " << endl << std::flush;

		try
		{
			// get reader interface outside loop
			PeriodicStreamReader ThreadPeriodicEventReader;

			// Context for the client. It could be used to convey extra information to
			// the server and/or tweak certain RPC behaviors.
			ClientContext context;

			// setup filter object for testing
			PeriodicDataRequest PeriodicDataTestSetup;

			PeriodicDataTestSetup.set_periodicinterval_ms(pClientApp->GetPeriodicRate());
			PeriodicDataTestSetup.set_senddeltaupdates(pClientApp->GetPeriodicDeltas());

			for(int i = 1; i <= pClientApp->GetStreamFilterRange(); i++)
			{
				ParameterData InitialParameterData;
				InitialParameterData.set_uid64(i);
				InitialParameterData.set_displayvalue(i);
				(*PeriodicDataTestSetup.mutable_periodicinitialvalues())[i] = InitialParameterData;

				cout << "MirrorReadPeriodicStream Thread [" << ThreadID << "] Setting Initial Periodic Data " << i
				     << endl
				     << std::flush;
			}
			cout << "----------------------------------------------------------------" << endl;

			cout << "MirrorReadPeriodicStream Thread [" << ThreadID << "] Update Rate "
			     << PeriodicDataTestSetup.periodicinterval_ms() << endl
			     << std::flush;
			cout << "MirrorReadPeriodicStream Thread [" << ThreadID << "] Total Elements to Update "
			     << PeriodicDataTestSetup.periodicinitialvalues().size() << endl
			     << std::flush;
			cout << "----------------------------------------------------------------" << endl;

			pClientApp->GetPeriodicDataStream(context, PeriodicData TestSetup, ThreadPeriodicEventReader);

			uint32_t SampleCount           = 0;
			uint32_t const SampleCountQuit = pClientApp->GetStreamClientMaxUpdates();
			bool const ShowReadValues      = pClientApp->GetShowReadValues();
			// bool TossFirstTiming = true;

#ifdef DEBUG_TIMING
			// time sample receipt
			struct timespec monotime;
			clock_gettime(CLOCK_MONOTONIC, &monotime);
#endif

			// print column headers if in show value mode
			if(ShowReadValues)
			{
				cout << "Count | UID64 | String | Float" << endl;
			}

			// blocked read
			// until stream closed

			struct timespec prevloopreadtime;
			clock_gettime(CLOCK_MONOTONIC, &prevloopreadtime);

			ParameterDataGroupMessage PeriodicUpdateStreamedData;
			while(ThreadPeriodicEventReader->Read(&PeriodicUpdateStreamedData))
			{
				SampleCount++;

				struct timespec thisloopreadtime;
				clock_gettime(CLOCK_MONOTONIC, &thisloopreadtime);
				uint64_t nsfromprevious = TimeDiffNS(thisloopreadtime, prevloopreadtime);
				prevloopreadtime        = thisloopreadtime;

				for(int32_t FindParm = 0; FindParm < PeriodicUpdateStreamedData.parameterdatagroup_size(); FindParm++)
				{
					::CDA::ParameterConsumer::ParameterData PublisherStreamedData =
					    PeriodicUpdateStreamedData.parameterdatagroup(FindParm);
					// if (PublisherStreamedData.uid64() == 1)
					{
						if(ShowReadValues)
						{
							string RawBinaryValue = PublisherStreamedData.binaryvalue();
							string BinaryDump     = "0x";
							for(uint32_t i = 0; i < RawBinaryValue.size(); i++)
							{
								char HexPair[2 + 1];
								HexPair[2] = 0x00;

								snprintf(HexPair, 3, "%02X", RawBinaryValue[i]);
								BinaryDump += HexPair;
							}
							cout << "Thread [" << ThreadID << "] " << SampleCount << "|"
							     << PublisherStreamedData.uid64() << "|" << PublisherStreamedData.displaystring() << "|"
							     << PublisherStreamedData.displayvalue() << "|" << BinaryDump << endl;
						}
					}
				}
				if(SampleCountQuit > 0)
				{
					if(SampleCount >= SampleCountQuit)
					{
						cout << "MirrorReadPeriodicStream: Exit Max Reads Exceeded " << SampleCountQuit << endl;
						break;  // out of while
					}
				}

				cout << "MirrorReadPeriodicStream: Loop=" << SampleCount
				     << " | Delay=" << (nsfromprevious / 1000 / 1000)
				     << "ms | Parms=" << PeriodicUpdateStreamedData.parameterdatagroup_size() << endl;
			}
		}
		catch(...)
		{
			cout << "Event Reader Exception Caught Thread [" << ThreadID << "] " << endl << std::flush;
		}

		cout << "End MirrorReadPeriodicStream Thread [" << ThreadID << "]" << endl << std::flush;
	}

	long GetStreamClientMaxUpdates() const
	{
		return m_StreamClientMaxUpdates;
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

	long GetPeriodicRate() const
	{
		return m_PeriodicRate;
	}

	bool GetPeriodicDeltas() const
	{
		return m_send_deltas;
	}

	void SetStreamClientMaxUpdates(long StreamClientMaxUpdates)
	{
		m_StreamClientMaxUpdates = StreamClientMaxUpdates;
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

	void SetPeriodicRate(long PeriodicRate)
	{
		m_PeriodicRate = PeriodicRate;
	}

	void SetPeriodicDeltas(bool send_deltas)
	{
		m_send_deltas = send_deltas;
	}

private:
	long m_StreamClientMaxUpdates;
	long m_StreamFilterRange;
	bool m_ShowReadValues;
	long m_PeriodicRate;
	bool m_send_deltas;
	bool m_TestReadSequential;
	std::unique_ptr<ParameterConsumer::Stub> ParameterConsumer_stub_;
	ParameterProducer m_Producer;
};

int main(int argc, char** argv)
{
	// Max String length = "192.168.100.123:50051"
	int const MAX_SERVER_ADDRESS = 21;
	int const MAX_RATE_CHARS     = 10;

	bool get_stream           = false;
	long stream_max_rx_count  = -1;
	long stream_filter_range  = -1;
	bool show_read_values     = false;
	bool test_read_sequential = false;
	long periodic_update_rate = -1;
	bool send_deltas          = false;

	// variable for server_address
	std::string server_address("127.0.0.1:50051");

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
			else if(strncmp(argv[i - 1], "--subscribe", 11) == 0)
			{
				get_stream = true;
				cout << "--subscribe= specified true " << endl;
			}
			else if(strncmp(argv[i - 1], "--streamclientmax=", 18) == 0)
			{
				char rate[MAX_RATE_CHARS + 1];
				char* dontcare;
				strncpy(rate, (argv[i - 1]) + 18, MAX_RATE_CHARS);
				stream_max_rx_count = strtol(rate, &dontcare, 10);
				cout << "--streamclientmax= specified " << stream_max_rx_count << endl;
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
			else if(strncmp(argv[i - 1], "--periodic=", 11) == 0)
			{
				char rate[MAX_RATE_CHARS + 1];
				char* dontcare;
				strncpy(rate, (argv[i - 1]) + 11, MAX_RATE_CHARS);
				periodic_update_rate = strtol(rate, &dontcare, 10);
				cout << "--periodic= specified " << periodic_update_rate << endl;
			}
			else if(strncmp(argv[i - 1], "--deltas", 8) == 0)
			{
				cout << "--deltas - specified TRUE" << endl;
				send_deltas = true;
			}

			else if(strncmp(argv[i - 1], "--help", 6) == 0)
			{
				cout << "CDAMirrorClientReader\n--ip=<IPAddr:Port>\n--subscribe - spin thread to read stream publisher "
				        "updates\n--streamclientmax=<quit client stream after # updates>\n--streamfilterrange=<filter "
				        "UIDs from 1 this value>\n--showreadvalues specifies show raw values\n--testreadsequential "
				        "test each double val is +1 from previous (no missed message)\n--periodic=<ms periodic "
				        "update>\n--deltas - send delta updates in periodic update\n--help Display This Usage Text"
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

	// disable compression
	// GRPC_COMPRESS_GZIP
	::grpc::ChannelArguments args;
	args.SetCompressionAlgorithm(GRPC_COMPRESS_NONE);

	CDAMirrorClientReaderImpl mirrorClientReader(
	    grpc::CreateCustomChannel(server_address, grpc::InsecureChannelCredentials(), args)
	);

	// setup stream max count for testing
	mirrorClientReader.SetStreamClientMaxUpdates(stream_max_rx_count);
	mirrorClientReader.SetStreamFilterRange(stream_filter_range);
	mirrorClientReader.SetShowReadValues(show_read_values);
	mirrorClientReader.SetTestReadSequential(test_read_sequential);
	mirrorClientReader.SetPeriodicRate(periodic_update_rate);
	mirrorClientReader.SetPeriodicDeltas(send_deltas);

	if(mirrorClientReader.GetStreamClientMaxUpdates() > 0)
	{
		printf("StreamMaxUpdates %li\n", mirrorClientReader.GetStreamClientMaxUpdates());
	}
	else
	{
		printf("StreamMaxUpdates Not Specified\n");
	}

	if(mirrorClientReader.GetStreamFilterRange() > 0)
	{
		printf("StreamFilterRange 1-%li\n", mirrorClientReader.GetStreamFilterRange());
	}
	else
	{
		printf("StreamFilterRange Not Specified\n");
	}

	if(mirrorClientReader.GetShowReadValues())
	{
		printf("ShowReadValues TRUE\n");
	}
	else
	{
		printf("ShowReadValues FALSE\n");
	}

	if(mirrorClientReader.GetPeriodicRate() > 0)
	{
		printf("Periodic Update %li\n", mirrorClientReader.GetPeriodicRate());
	}
	else
	{
		printf("Periodic Rate Not Specified\n");
	}

	if(mirrorClientReader.GetPeriodicDeltas())
	{
		printf("Periodic Sending with Deltas\n");
	}
	else
	{
		printf("Periodic Sending Full\n");
	}

	// spin client thread to do blocked reads with grpc reader object
	if(get_stream)
	{
		printf("Reading ParameterData Stream\n");
		thread ProducerUpdateStreamThread(CDAMirrorClientReaderImpl::MirrorReadUpdateStream, &mirrorClientReader);

		// stay alive while thread running
		ProducerUpdateStreamThread.join();
	}
	else
	{
		printf("Ignoring Reading Stream\n");
	}

	// spin client thread to do blocked reads with grpc reader object
	if(mirrorClientReader.GetPeriodicRate() > 0)
	{
		printf("Reading Periodic ParameterDataGroup Stream\n");
		thread PeriodicStreamThread(CDAMirrorClientReaderImpl::MirrorReadPeriodicStream, &mirrorClientReader);

		// stay alive while thread running
		PeriodicStreamThread.join();
	}
	else
	{
		printf("Ignoring Periodic Reading Stream\n");
	}

	return 0;
}
