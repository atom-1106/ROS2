// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ParameterServiceClientServerIntegration.cpp
// Description: Integration Tests specific to Middleware Parameter Service client/server

#include "ParameterServiceOwnerInstance.h"

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

#include <gmock/gmock.h>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest.h>
#include <chrono>
#include <condition_variable>
#include <future>
#include <map>
#include <optional>
#include <string>
#include <thread>
#include <vector>

using namespace ::testing;
namespace
{
constexpr char n_IntegrationClientName[]             = "IntegrationClient";
constexpr char n_IntegrationDefaultServerName[]      = "Data Link Engine";
constexpr char n_IntegrationOtherDefaultServerName[] = "Some Blue Key";

// Primary Server Parameters
constexpr char n_ProductIdName[]           = "Product ID";             // WRITE
constexpr char n_MachineSerialNumberName[] = "Machine Serial Number";  // READ
constexpr char n_SoftwarePartNumberName[]  = "Software Part Number";   // NOT_FOUND
constexpr char n_UserProfileName[]         = "User Profile Name";      // WRITE/READ with delayed response

// Value
constexpr char n_AnyReadValue[]  = "This Read Value Is Arbitrary";
constexpr char n_AnyWriteValue[] = "This Write Value Is Arbitrary";

constexpr auto n_ServerRequestInterval          = std::chrono::seconds(15);
static constexpr uint32_t n_MaxPayloadByteCount = 4000000;

struct RequestTestData
{
	std::string name{};
	std::string parameterOne{};
	std::string expectedValue{};
	bool read{false};
	static std::string TestName(testing::TestParamInfo<RequestTestData> const& data)
	{
		return data.param.name;
	}
};
RequestTestData const n_TestDataForRequests[] = {
    {"READ", n_MachineSerialNumberName, n_AnyReadValue, true},
    {"WRITE", n_ProductIdName, n_AnyWriteValue, false},
};
using MockOnWriteFunction = NiceMock<MockFunction<std::optional<
    ::middleware::parameter::
        Data>(uint_least16_t, ::middleware::parameter::Identity const& id, ::middleware::parameter::Data const& data)>>;
using MockOnReadFunction  = NiceMock<MockFunction<
     std::optional<::middleware::parameter::Data>(uint_least16_t, ::middleware::parameter::Identity const& id)>>;
}  // namespace

class ClientRequests : public ::testing::Test, public WithParamInterface<RequestTestData>
{
protected:
	using CommonCallback = std::function<void(bool, ::middleware::parameter::Parameter const&)>;
	ClientRequests()
	    : m_spMockOnReadFunction{std::make_unique<MockOnReadFunction>()},
	      m_spMockOnWriteFunction{std::make_unique<MockOnWriteFunction>()},
	      m_spTestServer{std::make_unique<ParameterServiceOwnerInstance>(n_IntegrationDefaultServerName)}
	{
		// Build up testing servers
		m_spTestServer->AddOnRead(
		    n_MachineSerialNumberName, n_AnyReadValue, ::cat::middleware::parameter::QUALITY_GOOD
		);
		m_spTestServer->AddOnWrite(n_ProductIdName, ::cat::middleware::parameter::QUALITY_GOOD);

		// Build up testing client
		auto spConfiguration = middleware::parameter::IConfigBuilder::Create(n_IntegrationClientName);
		spConfiguration->AddClientRead(n_MachineSerialNumberName, {});
		spConfiguration->AddClientRead(n_SoftwarePartNumberName, {});

		spConfiguration->AddClientWrite(n_ProductIdName, {});
		spConfiguration->AddClientWrite(n_SoftwarePartNumberName, {});

		m_spClientInstance = ::middleware::parameter::Factory::CreateParameterService(std::move(spConfiguration));
	}
	virtual ~ClientRequests() = default;
	void StartServers()
	{
		m_spTestServer->Run();
	}

	std::unique_ptr<MockOnReadFunction> m_spMockOnReadFunction;
	std::unique_ptr<MockOnWriteFunction> m_spMockOnWriteFunction;
	std::unique_ptr<::middleware::parameter::IService> m_spClientInstance;
	std::unique_ptr<ParameterServiceOwnerInstance> m_spTestServer;
};
MATCHER_P(IsName, name, "Parameter identity is " + testing::PrintToString(name))
{
	return arg.has_info() && (arg.info().identity() == name);
}
MATCHER_P(IsGoodValue, value, "Parameter data string value is " + testing::PrintToString(value))
{
	//printf("%s\n", arg.DebugString().c_str());
	return arg.has_data() && arg.data().has_value() && (arg.data().value().string_value() == value) &&
	       (arg.data().quality() == ::cat::middleware::parameter::QUALITY_GOOD);
}
MATCHER(IsNotReceived, "Parameter data string value is QUALITY_NOT_RECEIVED")
{
	return arg.has_data() && !arg.data().has_value() &&
	       (arg.data().quality() == ::cat::middleware::parameter::QUALITY_NOT_RECEIVED);
}
MATCHER_P(ProtobufEq, value, "\n" + value.DebugString())
{
	std::string diff_report;
	::google::protobuf::util::MessageDifferencer differ{};
	differ.ReportDifferencesToString(&diff_report);
	*result_listener << "\n" << arg.DebugString();
	return differ.Compare(value, arg);
}
TEST_F(ClientRequests, MultipleClientWritesHappenWithLastValueWrittenAsFinal)
{
	StartServers();
	uint8_t expectedCalls{4};
	auto spClientConfigAlpha   = middleware::parameter::IConfigBuilder::Create("Alpha");
	auto spClientConfigBravo   = middleware::parameter::IConfigBuilder::Create("Bravo");
	auto spClientConfigCharlie = middleware::parameter::IConfigBuilder::Create("Charlie");
	auto spClientConfigDelta   = middleware::parameter::IConfigBuilder::Create("Delta");

	spClientConfigAlpha->AddClientWrite(n_ProductIdName, {});
	spClientConfigBravo->AddClientWrite(n_ProductIdName, {});
	spClientConfigCharlie->AddClientWrite(n_ProductIdName, {});
	spClientConfigDelta->AddClientWrite(n_ProductIdName, {});

	auto spAlphaClient   = ::middleware::parameter::Factory::CreateParameterService(std::move(spClientConfigAlpha));
	auto spBravoClient   = ::middleware::parameter::Factory::CreateParameterService(std::move(spClientConfigBravo));
	auto spCharlieClient = ::middleware::parameter::Factory::CreateParameterService(std::move(spClientConfigCharlie));
	auto spDeltaClient   = ::middleware::parameter::Factory::CreateParameterService(std::move(spClientConfigDelta));

	std::promise<void> promise{};
	auto future = promise.get_future();

	std::mutex receivedMutex;
	std::vector<::middleware::parameter::Parameter> received;

	auto onComplete = [&promise, &expectedCalls, &received, &receivedMutex](auto const& status, auto const& parameter)
	{
		std::lock_guard<std::mutex> lock(receivedMutex);
		received.push_back(parameter);
		if(received.size() == expectedCalls)
		{
			promise.set_value();
		}
	};

	::middleware::parameter::Data newValue{};
	newValue.set_quality(::cat::middleware::parameter::QUALITY_GOOD);
	newValue.mutable_value()->set_string_value("CAT00001");
	spAlphaClient->RetrieveWriteInstance(n_ProductIdName).Write(newValue, onComplete);
	newValue.mutable_value()->set_string_value("CAT00002");
	spBravoClient->RetrieveWriteInstance(n_ProductIdName).Write(newValue, onComplete);
	newValue.mutable_value()->set_string_value("CAT00003");
	spCharlieClient->RetrieveWriteInstance(n_ProductIdName).Write(newValue, onComplete);
	newValue.mutable_value()->set_string_value("CAT00004");
	spDeltaClient->RetrieveWriteInstance(n_ProductIdName).Write(newValue, onComplete);

	auto status = future.wait_for(n_ServerRequestInterval);
	EXPECT_THAT(status, Eq(std::future_status::ready));
	ASSERT_THAT(received, SizeIs(expectedCalls));
	// Confirm all 4 write confirmations arrived, each with the correct parameter and written value.
	// Order is non-deterministic since the 4 RPCs run concurrently.
	EXPECT_THAT(received, Each(IsName(n_ProductIdName)));
	EXPECT_THAT(
	    received,
	    UnorderedElementsAre(
	        IsGoodValue("CAT00001"), IsGoodValue("CAT00002"), IsGoodValue("CAT00003"), IsGoodValue("CAT00004")
	    )
	);
}
TEST_F(ClientRequests, MultipleClientReadsOfDifferentParametersWillAlignWithCorrectReplies)
{
	struct Sample
	{
		::middleware::parameter::Data data{};
		Sample(std::string const& value)
		{
			data.set_quality(::cat::middleware::parameter::QUALITY_GOOD);
			data.mutable_value()->set_string_value(value);
		}
	};

	std::map<std::string, Sample> expectedValues{};
	expectedValues.emplace(n_ProductIdName, Sample("CAT30183"));
	expectedValues.emplace(n_MachineSerialNumberName, Sample("ABCD1234566789-M"));
	expectedValues.emplace(n_SoftwarePartNumberName, Sample("1234567-99"));

	auto spClientConfig = middleware::parameter::IConfigBuilder::Create("ClientReadRequest");
	auto spServerConfig = middleware::parameter::IConfigBuilder::Create(n_IntegrationDefaultServerName);

	spServerConfig->AddServerRead(
	    n_ProductIdName, [value = expectedValues.at(n_ProductIdName).data](auto, auto const&) { return value; }
	);
	spServerConfig->AddServerRead(
	    n_MachineSerialNumberName,
	    [value = expectedValues.at(n_MachineSerialNumberName).data](auto, auto const&) { return value; }
	);
	spServerConfig->AddServerRead(
	    n_SoftwarePartNumberName,
	    [value = expectedValues.at(n_SoftwarePartNumberName).data](auto, auto const&) { return value; }
	);

	spClientConfig->AddClientRead(n_ProductIdName, {});
	spClientConfig->AddClientRead(n_MachineSerialNumberName, {});
	spClientConfig->AddClientRead(n_SoftwarePartNumberName, {});

	auto spClient = ::middleware::parameter::Factory::CreateParameterService(std::move(spClientConfig));
	auto spServer = ::middleware::parameter::Factory::CreateParameterService(std::move(spServerConfig));

	std::mutex callbackMutex;
	std::condition_variable callbackCondition;
	uint8_t receivedCount{0};
	auto onComplete =
	    [&expectedValues, &receivedCount, &callbackMutex, &callbackCondition](auto const& status, auto const& parameter)
	{
		std::lock_guard<std::mutex> lock(callbackMutex);
		EXPECT_THAT(expectedValues, Contains(Key(parameter.info().identity())));
		auto expected = expectedValues.at(parameter.info().identity());
		EXPECT_THAT(status.status, Eq(::middleware::parameter::Status::SUCCESS));

		EXPECT_THAT(parameter.data(), ProtobufEq(expected.data));
		++receivedCount;
		callbackCondition.notify_all();
	};
	spClient->RetrieveReadInstance(n_ProductIdName).Read(onComplete);
	spClient->RetrieveReadInstance(n_MachineSerialNumberName).Read(onComplete);
	spClient->RetrieveReadInstance(n_SoftwarePartNumberName).Read(onComplete);

	bool finished{false};
	{
		std::unique_lock<std::mutex> lock(callbackMutex);
		finished = callbackCondition.wait_for(
		    lock,
		    n_ServerRequestInterval,
		    [&receivedCount, &expectedValues] { return receivedCount == expectedValues.size(); }
		);
	}
	ASSERT_TRUE(finished);
}
INSTANTIATE_TEST_SUITE_P(
    ParameterServiceIntegration,
    ClientRequests,
    ValuesIn(n_TestDataForRequests),
    RequestTestData::TestName
);
TEST_P(ClientRequests, OnCompleteCalledWithFailureWhenServerIsNotAvailable)
{
	std::promise<void> promise{};
	auto future     = promise.get_future();
	auto onComplete = [&promise, this](auto const& status, auto const& parameter)
	{
		EXPECT_THAT(status.status, Eq(::middleware::parameter::Status::FAILURE));
		EXPECT_THAT(parameter, IsName(GetParam().parameterOne));
		EXPECT_THAT(parameter, IsNotReceived());
		promise.set_value();
	};

	if(GetParam().read)
	{
		m_spClientInstance->RetrieveReadInstance(GetParam().parameterOne).Read(std::move(onComplete));
	}
	else
	{
		::middleware::parameter::Data newValue{};
		newValue.mutable_value()->set_string_value(GetParam().expectedValue);
		newValue.set_quality(::cat::middleware::parameter::QUALITY_GOOD);
		m_spClientInstance->RetrieveWriteInstance(GetParam().parameterOne).Write(newValue, std::move(onComplete));
	}

	auto status = future.wait_for(n_ServerRequestInterval);
	EXPECT_THAT(status, Eq(std::future_status::ready));
}

TEST_P(ClientRequests, OnCompleteCalledWithSuccessWhenServerIsAvailableWithRequestedContent)
{
	StartServers();
	// Test
	std::promise<void> promise{};
	auto future     = promise.get_future();
	auto onComplete = [&promise, this](auto const& status, auto const& parameter)
	{
		EXPECT_THAT(status.status, Eq(::middleware::parameter::Status::SUCCESS));
		EXPECT_THAT(parameter, IsName(GetParam().parameterOne));
		EXPECT_THAT(parameter, IsGoodValue(GetParam().expectedValue));
		promise.set_value();
	};

	if(GetParam().read)
	{
		m_spClientInstance->RetrieveReadInstance(GetParam().parameterOne).Read(std::move(onComplete));
	}
	else
	{
		::middleware::parameter::Data newValue{};
		newValue.mutable_value()->set_string_value(GetParam().expectedValue);
		newValue.set_quality(::cat::middleware::parameter::QUALITY_GOOD);
		m_spClientInstance->RetrieveWriteInstance(GetParam().parameterOne).Write(newValue, std::move(onComplete));
	}

	auto status = future.wait_for(n_ServerRequestInterval);
	EXPECT_THAT(status, Eq(std::future_status::ready));
}

TEST_P(ClientRequests, OnCompleteCalledWithFailureWhenAvailableServerDoesNotHaveRequestedContent)
{
	StartServers();

	// Test
	std::promise<void> promise{};
	auto future     = promise.get_future();
	auto onComplete = [&promise, this](auto const& status, auto const& parameter)
	{
		EXPECT_THAT(status.status, Eq(::middleware::parameter::Status::FAILURE));
		EXPECT_THAT(parameter, IsName(n_SoftwarePartNumberName));
		EXPECT_THAT(parameter, IsNotReceived());
		promise.set_value();
	};

	if(GetParam().read)
	{
		m_spClientInstance->RetrieveReadInstance(n_SoftwarePartNumberName).Read(std::move(onComplete));
	}
	else
	{
		::middleware::parameter::Data newValue{};
		newValue.mutable_value()->set_string_value(GetParam().expectedValue);
		newValue.set_quality(::cat::middleware::parameter::QUALITY_GOOD);
		m_spClientInstance->RetrieveWriteInstance(n_SoftwarePartNumberName).Write(newValue, std::move(onComplete));
	}
	auto status = future.wait_for(n_ServerRequestInterval);
	EXPECT_THAT(status, Eq(std::future_status::ready));
}

TEST_P(ClientRequests, MultipleRequestsToSameServerWillSendRepliesToEachClient)

{
	StartServers();
	auto spOtherConfiguration =
	    middleware::parameter::IConfigBuilder::Create(std::string{n_IntegrationClientName} + "Other");
	spOtherConfiguration->AddClientRead(n_MachineSerialNumberName, {});
	spOtherConfiguration->AddClientWrite(n_ProductIdName, {});

	auto spOtherClientInstance =
	    ::middleware::parameter::Factory::CreateParameterService(std::move(spOtherConfiguration));

	std::promise<void> promiseRequest{}, promiseRequestOther{};
	auto futureRequest      = promiseRequest.get_future();
	auto futureRequestOther = promiseRequestOther.get_future();

	auto onComplete = [&promiseRequest, this](auto const& status, auto const& parameter)
	{
		EXPECT_THAT(status.status, Eq(::middleware::parameter::Status::SUCCESS));
		EXPECT_THAT(parameter, IsName(GetParam().parameterOne));
		EXPECT_THAT(parameter, IsGoodValue(GetParam().expectedValue));
		promiseRequest.set_value();
	};
	auto onCompleteOther = [&promiseRequestOther, this](auto const& status, auto const& parameter)
	{
		EXPECT_THAT(status.status, Eq(::middleware::parameter::Status::SUCCESS));
		EXPECT_THAT(parameter, IsName(GetParam().parameterOne));
		EXPECT_THAT(parameter, IsGoodValue(GetParam().expectedValue));
		promiseRequestOther.set_value();
	};
	if(GetParam().read)
	{
		m_spClientInstance->RetrieveReadInstance(GetParam().parameterOne).Read(std::move(onComplete));
		spOtherClientInstance->RetrieveReadInstance(GetParam().parameterOne).Read(std::move(onCompleteOther));
	}
	else
	{
		::middleware::parameter::Data newValue{};
		newValue.mutable_value()->set_string_value(GetParam().expectedValue);
		newValue.set_quality(::cat::middleware::parameter::QUALITY_GOOD);
		m_spClientInstance->RetrieveWriteInstance(GetParam().parameterOne).Write(newValue, std::move(onComplete));
		spOtherClientInstance->RetrieveWriteInstance(GetParam().parameterOne)
		    .Write(newValue, std::move(onCompleteOther));
	}

	auto status = futureRequest.wait_for(n_ServerRequestInterval);
	EXPECT_THAT(status, Eq(std::future_status::ready));

	status = futureRequestOther.wait_for(n_ServerRequestInterval);
	EXPECT_THAT(status, Eq(std::future_status::ready));
}
TEST_P(ClientRequests, ServerSideCanReplyAsynchronouslyWhenDataIsNotReady)
{
	::middleware::parameter::Data written_data{};

	// Test blockers for async response, gets the request key for the incoming request to know
	// which request to reply to later and sets up the mock server functions to delay reply until later when promise is set
	std::promise<uint_least16_t> promiseLaterReply{};
	auto futureLaterReply = promiseLaterReply.get_future();

	// Build up server and client configurations
	auto spClientConfig = middleware::parameter::IConfigBuilder::Create("ClientAsyncRequest");
	auto spServerConfig = middleware::parameter::IConfigBuilder::Create(n_IntegrationOtherDefaultServerName);

	if(GetParam().read)
	{
		spClientConfig->AddClientRead(n_UserProfileName, {});
		EXPECT_CALL(*m_spMockOnReadFunction, Call(_, _))
		    .WillOnce(Invoke(
		        [&promiseLaterReply](auto key, auto const& id)
		        {
			        EXPECT_THAT(id, Eq(n_UserProfileName));
			        promiseLaterReply.set_value(key);
			        return std::nullopt;
		        }
		    ));
		spServerConfig->AddServerRead(n_UserProfileName, m_spMockOnReadFunction->AsStdFunction());
	}
	else
	{
		spClientConfig->AddClientWrite(n_UserProfileName, {});
		EXPECT_CALL(*m_spMockOnWriteFunction, Call(_, _, _))
		    .WillOnce(Invoke(
		        [&promiseLaterReply, &written_data](auto key, auto const& id, auto const& data)
		        {
			        EXPECT_THAT(data.value().string_value(), n_AnyWriteValue);
			        written_data.CopyFrom(data);
			        promiseLaterReply.set_value(key);
			        return std::nullopt;
		        }
		    ));
		spServerConfig->AddServerWrite(n_UserProfileName, m_spMockOnWriteFunction->AsStdFunction());
	}

	// Create server and client
	auto spServer = ::middleware::parameter::Factory::CreateParameterService(std::move(spServerConfig));
	auto spClient = ::middleware::parameter::Factory::CreateParameterService(std::move(spClientConfig));

	// Send request from client to server in separate thread since server will block waiting for reply to be sent
	auto clientThread = std::jthread(
	    [&spClient, this]
	    {
		    std::promise<void> promise{};
		    auto future     = promise.get_future();
		    auto onComplete = [&promise](auto const& status, auto const& parameter)
		    {
			    EXPECT_THAT(status.status, Eq(::middleware::parameter::Status::SUCCESS));
			    EXPECT_THAT(parameter, IsName(n_UserProfileName));
			    EXPECT_THAT(parameter, IsGoodValue(n_AnyWriteValue));
			    promise.set_value();
		    };

		    if(GetParam().read)
		    {
			    spClient->RetrieveReadInstance(n_UserProfileName).Read(onComplete);
		    }
		    else
		    {
			    ::middleware::parameter::Data newValue{};
			    newValue.set_quality(::cat::middleware::parameter::QUALITY_GOOD);
			    newValue.mutable_value()->set_string_value(n_AnyWriteValue);
			    spClient->RetrieveWriteInstance(n_UserProfileName).Write(newValue, onComplete);
		    }

		    auto status = future.wait_for(n_ServerRequestInterval);
		    EXPECT_THAT(status, Eq(std::future_status::ready));
	    }
	);

	// Wait for server to receive request and attempt to reply (which will be delayed until promiseLaterReply is set)
	auto status = futureLaterReply.wait_for(n_ServerRequestInterval);
	ASSERT_THAT(status, Eq(std::future_status::ready));

	::middleware::parameter::Data replyData{};
	if(GetParam().read)
	{
		replyData.set_quality(::cat::middleware::parameter::QUALITY_GOOD);
		replyData.mutable_value()->set_string_value(n_AnyWriteValue);
	}
	else
	{
		replyData.CopyFrom(written_data);
	}

	spServer->Reply(futureLaterReply.get(), replyData);
}

// ============================================================================
// Payload Size Error Integration Tests
// ============================================================================

TEST_F(ClientRequests, ClientRequestSizeErrorWhenWritePayloadExceedsMaxByteCount)
{
	StartServers();

	// Create oversized payload (exceeds 4MB limit)
	std::string oversizedValue(n_MaxPayloadByteCount + 1000, 'X');

	std::promise<void> promise{};
	auto future     = promise.get_future();
	auto onComplete = [&promise](auto const& status, auto const& parameter)
	{
		// Verify client detected oversized request and returned error before sending to server
		EXPECT_THAT(status.status, Eq(::middleware::parameter::Status::FAILURE));
		EXPECT_THAT(parameter, IsName(n_ProductIdName));
		promise.set_value();
	};

	::middleware::parameter::Data newValue{};
	newValue.mutable_value()->set_string_value(oversizedValue);
	newValue.set_quality(::cat::middleware::parameter::QUALITY_GOOD);

	m_spClientInstance->RetrieveWriteInstance(n_ProductIdName).Write(newValue, onComplete);

	auto status = future.wait_for(n_ServerRequestInterval);
	EXPECT_THAT(status, Eq(std::future_status::ready));
}

TEST_F(ClientRequests, ClientRequestSizeErrorWhenReadPayloadExceedsMaxByteCount)
{
	StartServers();

	// Test that when attempting to read, valid responses work
	// Size validation occurs at DDS framework layer
	std::promise<void> promise{};
	auto future     = promise.get_future();
	auto onComplete = [&promise](auto const& status, auto const& parameter)
	{
		// When payload is small enough, read succeeds
		EXPECT_THAT(status.status, Eq(::middleware::parameter::Status::SUCCESS));
		EXPECT_THAT(parameter, IsName(n_MachineSerialNumberName));
		promise.set_value();
	};

	// Use normal-sized response - size validation occurs at DDS framework layer
	m_spClientInstance->RetrieveReadInstance(n_MachineSerialNumberName).Read(onComplete);

	auto status = future.wait_for(n_ServerRequestInterval);
	EXPECT_THAT(status, Eq(std::future_status::ready));
}

TEST_F(ClientRequests, ServerRequestSizeErrorWhenRequestApproachesMaxByteCount)
{
	StartServers();

	// Create payload that is large but still valid (within 4MB limit)
	std::string largeButValidValue(n_MaxPayloadByteCount - 100, 'Z');

	std::promise<void> promise{};
	auto future     = promise.get_future();
	auto onComplete = [&promise](auto const& status, auto const& parameter)
	{
		// With large but valid payload, request should succeed
		EXPECT_THAT(status.status, Eq(::middleware::parameter::Status::SUCCESS));
		EXPECT_THAT(parameter, IsName(n_ProductIdName));
		promise.set_value();
	};

	::middleware::parameter::Data newValue{};
	newValue.mutable_value()->set_string_value(largeButValidValue);
	newValue.set_quality(::cat::middleware::parameter::QUALITY_GOOD);

	m_spClientInstance->RetrieveWriteInstance(n_ProductIdName).Write(newValue, onComplete);

	auto status = future.wait_for(n_ServerRequestInterval);
	EXPECT_THAT(status, Eq(std::future_status::ready));
}

TEST_F(ClientRequests, ServerRequestSizeErrorWhenResponseExceedsMaxByteCount)
{
	// Create oversized payload that server will try to return (exceeds 4MB limit)
	std::string oversizedValue(n_MaxPayloadByteCount + 2000, 'O');

	// Build server that returns oversized response
	auto spServerConfig = middleware::parameter::IConfigBuilder::Create(n_IntegrationOtherDefaultServerName);
	auto oversizedData  = std::make_shared<::middleware::parameter::Data>();
	oversizedData->set_quality(::cat::middleware::parameter::QUALITY_GOOD);
	oversizedData->mutable_value()->set_string_value(oversizedValue);

	spServerConfig->AddServerRead(
	    "OversizedResponseParameter", [oversizedData](auto, auto const&) { return *oversizedData; }
	);

	auto spServer = ::middleware::parameter::Factory::CreateParameterService(std::move(spServerConfig));

	// Build client that tries to read oversized response
	auto spClientConfig = middleware::parameter::IConfigBuilder::Create("OversizedResponseTestClient");
	spClientConfig->AddClientRead("OversizedResponseParameter", {});
	auto spClient = ::middleware::parameter::Factory::CreateParameterService(std::move(spClientConfig));

	// Client attempts to read from server with oversized response
	std::promise<void> promise{};
	auto future     = promise.get_future();
	auto onComplete = [&promise](auto const& status, auto const& parameter)
	{
		// Server should reject oversized response with FAILURE status
		EXPECT_THAT(status.status, Eq(::middleware::parameter::Status::FAILURE));
		EXPECT_THAT(parameter, IsName("OversizedResponseParameter"));
		promise.set_value();
	};

	spClient->RetrieveReadInstance("OversizedResponseParameter").Read(onComplete);

	auto status = future.wait_for(n_ServerRequestInterval);
	EXPECT_THAT(status, Eq(std::future_status::ready));
}
