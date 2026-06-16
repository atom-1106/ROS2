// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareFrameworkServerRequestHandlerTest.cpp
// Description: Unit Tests for ServerRequestHandler class

#include "ServerRequestHandler.cpp"
#include "TestUtilities.h"
#include "mockEprosimaRpcRequest.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

using namespace ::testing;
namespace
{

static ::eprosima::fastdds::rtps::GuidPrefix_t SomeGuidPrefix()
{
	::eprosima::fastdds::rtps::GuidPrefix_t guidPrefix{};
	guidPrefix.value[0]  = 'T';
	guidPrefix.value[1]  = 'E';
	guidPrefix.value[2]  = 'S';
	guidPrefix.value[3]  = 'T';
	guidPrefix.value[4]  = 'C';
	guidPrefix.value[5]  = 'L';
	guidPrefix.value[6]  = 'I';
	guidPrefix.value[7]  = 'E';
	guidPrefix.value[8]  = 'N';
	guidPrefix.value[9]  = 'T';
	guidPrefix.value[10] = '0';
	guidPrefix.value[11] = '1';
	return guidPrefix;
}
constexpr auto n_ServerRequestInterval = std::chrono::seconds(15);
}  // namespace

class MiddlewareFrameworkServerRequestHandlerTest : public ::testing::Test
{
protected:
	MiddlewareFrameworkServerRequestHandlerTest()
	    : testClientGuid{SomeGuidPrefix(), 1}, m_spMockRequestInfo{std::make_unique<NiceMock<::mock::RpcRequest>>()}
	{
		// This is to appease cppcheck
		EXPECT_FALSE(testClientGuidText.empty());
		ON_CALL(*m_spMockRequestInfo, get_client_id())
		    .WillByDefault(Invoke([this] -> ::eprosima::fastdds::rtps::GUID_t const& { return testClientGuid; }));
	}
	virtual ~MiddlewareFrameworkServerRequestHandlerTest() = default;
	::eprosima::fastdds::rtps::GUID_t testClientGuid;
	// The prefix converts to hex with the domain ip appended
	std::string testClientGuidText{"54.45.53.54.43.4c.49.45.4e.54.30.31|0.0.0.1"};
	std::unique_ptr<NiceMock<::mock::RpcRequest>> m_spMockRequestInfo;
};
TEST_F(MiddlewareFrameworkServerRequestHandlerTest, DeathTestConstructorPassedNullptrOnReadCallback)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";

	auto callback = [](auto, auto const& request)
	{
		return ::middleware::Reply{};
	};

	ASSERT_DEATH(std::make_unique<::middleware::dds::ServerRequestHandler>(nullptr, callback), "Assertion .* failed");
}
TEST_F(MiddlewareFrameworkServerRequestHandlerTest, DeathTestConstructorPassedNullptrOnWriteCallback)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";

	auto callback = [](auto, auto const& request)
	{
		return ::middleware::Reply{};
	};

	ASSERT_DEATH(std::make_unique<::middleware::dds::ServerRequestHandler>(callback, nullptr), "Assertion .* failed");
}
TEST_F(MiddlewareFrameworkServerRequestHandlerTest, ReturnResponseWhenReadIsCalled)
{
	std::vector<uint8_t> anyReply{0x43, 0x92, 0x0A};
	std::vector<uint8_t> anyRequest{0x01, 0xAB, 0xC1};
	std::vector<uint8_t> received{};

	auto onRead = [&received, &anyReply](auto, auto const& request)
	{
		received = request.serialized_protobuf;

		::middleware::Reply reply{};
		reply.serialized_protobuf = anyReply;
		return reply;
	};
	auto onWrite = [](auto, auto const& request)
	{
		return ::middleware::Reply{};
	};

	auto spObjectUnderTest = std::make_unique<::middleware::dds::ServerRequestHandler>(onRead, onWrite);

	::cat::middleware::dds::TransferData request{};
	request.protobuf(anyRequest);
	auto reply = spObjectUnderTest->Read(*m_spMockRequestInfo, request);

	EXPECT_THAT(reply.protobuf(), Eq(anyReply));
}
TEST_F(MiddlewareFrameworkServerRequestHandlerTest, ReturnResponseWhenWriteIsCalled)
{
	std::vector<uint8_t> anyReply{0x43, 0x92, 0x0A};
	std::vector<uint8_t> anyRequest{0x01, 0xAB, 0xC1};
	std::vector<uint8_t> received{};

	auto onRead = [&received, &anyReply](auto, auto const& request)
	{
		return ::middleware::Reply{};
	};
	auto onWrite = [&received, &anyReply](auto, auto const& request)
	{
		received = request.serialized_protobuf;

		::middleware::Reply reply{};
		reply.serialized_protobuf = anyReply;
		return reply;
	};

	auto spObjectUnderTest = std::make_unique<::middleware::dds::ServerRequestHandler>(onRead, onWrite);

	::cat::middleware::dds::TransferData request{};
	request.protobuf(anyRequest);
	auto reply = spObjectUnderTest->Write(*m_spMockRequestInfo, request);

	EXPECT_THAT(reply.protobuf(), Eq(anyReply));
}
TEST_F(
    MiddlewareFrameworkServerRequestHandlerTest,
    ThrowRequestExceptionWithClientRequestSizeErrorCodeWhenReadRequestIsEmpty
)
{

	auto callback = [](auto, auto const& request)
	{
		return ::middleware::Reply{};
	};

	auto spObjectUnderTest = std::make_unique<::middleware::dds::ServerRequestHandler>(callback, callback);

	try
	{
		::cat::middleware::dds::TransferData request{};
		spObjectUnderTest->Read(*m_spMockRequestInfo, request);
	}
	catch(::cat::middleware::dds::RequestException const& exception)
	{
		std::stringstream expected_msg{};
		expected_msg << "Empty request was received from [" << testClientGuidText << "].";
		EXPECT_THAT(std::string{exception.what()}, Eq(expected_msg.str()));
		EXPECT_THAT(
		    exception.status_code(),
		    Eq(static_cast<uint32_t>(::middleware::StatusCode::STATUS_CODE_CLIENT_REQUEST_SIZE_ERROR))
		);
		EXPECT_TRUE(exception.request().protobuf().empty());
	}
	catch(...)
	{
		FAIL();
	}
}
TEST_F(
    MiddlewareFrameworkServerRequestHandlerTest,
    ThrowRequestExceptionWithClientRequestSizeErrorCodeWhenWriteRequestIsEmpty
)
{

	auto callback = [](auto, auto const& request)
	{
		return ::middleware::Reply{};
	};

	auto spObjectUnderTest = std::make_unique<::middleware::dds::ServerRequestHandler>(callback, callback);

	try
	{
		::cat::middleware::dds::TransferData request{};
		spObjectUnderTest->Write(*m_spMockRequestInfo, request);
	}
	catch(::cat::middleware::dds::RequestException const& exception)
	{
		std::stringstream expected_msg{};
		expected_msg << "Empty request was received from [" << testClientGuidText << "].";
		EXPECT_THAT(std::string{exception.what()}, Eq(expected_msg.str()));
		EXPECT_THAT(
		    exception.status_code(),
		    Eq(static_cast<uint32_t>(::middleware::StatusCode::STATUS_CODE_CLIENT_REQUEST_SIZE_ERROR))
		);
		EXPECT_TRUE(exception.request().protobuf().empty());
	}
	catch(...)
	{
		FAIL();
	}
}
TEST_F(
    MiddlewareFrameworkServerRequestHandlerTest,
    ThrowRequestExceptionWithServerInternalErrorCodeWhenOnReadCallbackThrowsInternally
)
{
	std::vector<uint8_t> anyRequest{0x01, 0xAB, 0xC1};
	// This is to appease cppcheck due to no analysis in try/catch
	EXPECT_FALSE(anyRequest.empty());
	auto callback = [](auto, auto const& request)
	{
		std::unordered_map<uint32_t, std::vector<uint8_t>> someData{};
		::middleware::Reply reply{};
		// Bad access will cause an exception from the callback
		reply.serialized_protobuf = someData.at(UINT32_MAX);
		return reply;
	};

	auto spObjectUnderTest = std::make_unique<::middleware::dds::ServerRequestHandler>(callback, callback);

	try
	{
		::cat::middleware::dds::TransferData request{};
		request.protobuf(anyRequest);
		spObjectUnderTest->Read(*m_spMockRequestInfo, request);
	}
	catch(::cat::middleware::dds::RequestException const& exception)
	{
		std::stringstream expected_msg{};
		expected_msg << "Client [" << testClientGuidText << "] on callback exception: unordered_map::at";
		EXPECT_THAT(std::string{exception.what()}, Eq(expected_msg.str()));
		EXPECT_THAT(
		    exception.status_code(),
		    Eq(static_cast<uint32_t>(::middleware::StatusCode::STATUS_CODE_SERVER_INTERNAL_ERROR))
		);
		EXPECT_THAT(exception.request().protobuf(), Eq(anyRequest));
	}
	catch(...)
	{
		FAIL();
	}
}
TEST_F(
    MiddlewareFrameworkServerRequestHandlerTest,
    ThrowRequestExceptionWithServerInternalErrorCodeWhenOnWriteCallbackThrowsInternally
)
{
	std::vector<uint8_t> anyRequest{0x01, 0xAB, 0xC1};
	// This is to appease cppcheck due to no analysis in try/catch
	EXPECT_FALSE(anyRequest.empty());

	auto callback = [](auto, auto const& request)
	{
		std::unordered_map<uint32_t, std::vector<uint8_t>> someData{};
		::middleware::Reply reply{};
		// Bad access will cause an exception from the callback
		reply.serialized_protobuf = someData.at(UINT32_MAX);
		return reply;
	};

	auto spObjectUnderTest = std::make_unique<::middleware::dds::ServerRequestHandler>(callback, callback);

	try
	{
		::cat::middleware::dds::TransferData request{};
		request.protobuf(anyRequest);
		spObjectUnderTest->Write(*m_spMockRequestInfo, request);
	}
	catch(::cat::middleware::dds::RequestException const& exception)
	{
		std::stringstream expected_msg{};
		expected_msg << "Client [" << testClientGuidText << "] on callback exception: unordered_map::at";
		EXPECT_THAT(std::string{exception.what()}, Eq(expected_msg.str()));
		EXPECT_THAT(
		    exception.status_code(),
		    Eq(static_cast<uint32_t>(::middleware::StatusCode::STATUS_CODE_SERVER_INTERNAL_ERROR))
		);
		EXPECT_THAT(exception.request().protobuf(), Eq(anyRequest));
	}
	catch(...)
	{
		FAIL();
	}
}
TEST_F(
    MiddlewareFrameworkServerRequestHandlerTest,
    ThrowRequestExceptionWithServerRequestSizeErrorWhenCallbackResponseIsGreaterThan4MB
)
{
	std::vector<uint8_t> anyRequest{0x01, 0xAB, 0xC1};
	auto onRead = [](auto, auto const& request)
	{
		::middleware::Reply reply{};
		reply.serialized_protobuf.resize(::middleware::max::n_MaxPayloadByteCount + 1);
		return reply;
	};
	auto onWrite = [](auto, auto const& request)
	{
		return ::middleware::Reply{};
	};

	auto spObjectUnderTest = std::make_unique<::middleware::dds::ServerRequestHandler>(onRead, onWrite);

	try
	{
		::cat::middleware::dds::TransferData request{};
		request.protobuf(anyRequest);
		spObjectUnderTest->Read(*m_spMockRequestInfo, request);
		FAIL();
	}
	catch(::cat::middleware::dds::RequestException const& exception)
	{
		EXPECT_THAT(
		    exception.status_code(),
		    Eq(static_cast<uint32_t>(::middleware::StatusCode::STATUS_CODE_SERVER_REQUEST_SIZE_ERROR))
		);
		EXPECT_THAT(exception.request().protobuf(), Eq(anyRequest));
	}
	catch(...)
	{
		FAIL();
	}
}
namespace
{
// Helper function to create test data
void SetupTestReply(middleware::Reply& reply)
{
	reply.serialized_protobuf   = {0x43, 0x92, 0x0A};
	reply.status.status_code    = ::middleware::StatusCode::STATUS_CODE_SUCCESS;
	reply.status.status_message = "Test Success";
}

// Helper function to wait and assert on future status
template <typename T>
void WaitAndAssertReady(std::future<T>& future, std::chrono::seconds timeout)
{
	auto status = future.wait_for(timeout);
	ASSERT_THAT(status, Eq(std::future_status::ready));
}
}  // namespace

TEST_F(
    MiddlewareFrameworkServerRequestHandlerTest,
    ReplyLaterWillReturnDataForReadRequestsWhenTheAssignedRequestKeyIsRepliedTo
)
{
	std::promise<uint64_t> callbackPromise{};
	auto callbackFuture = callbackPromise.get_future();
	std::vector<uint8_t> anyRequest{0x01, 0xAB, 0xC1};
	middleware::Reply anyReply{};
	SetupTestReply(anyReply);

	auto onRead = [&callbackPromise](auto key, auto const& request)
	{
		callbackPromise.set_value(key);
		return std::nullopt;
	};
	auto onWrite = [](auto, auto const& request)
	{
		return ::middleware::Reply{};
	};

	auto spObjectUnderTest = std::make_unique<::middleware::dds::ServerRequestHandler>(onRead, onWrite);

	::cat::middleware::dds::TransferData request{};
	request.protobuf(anyRequest);
	auto readFuture = std::async(
	    &::middleware::dds::ServerRequestHandler::Read, spObjectUnderTest.get(), std::ref(*m_spMockRequestInfo), request
	);

	WaitAndAssertReady(callbackFuture, n_ServerRequestInterval);
	spObjectUnderTest->Reply(callbackFuture.get(), anyReply);

	WaitAndAssertReady(readFuture, n_ServerRequestInterval);
	auto repliedTransferData = readFuture.get();
	EXPECT_THAT(repliedTransferData.protobuf(), Eq(anyReply.serialized_protobuf));
}
TEST_F(
    MiddlewareFrameworkServerRequestHandlerTest,
    ReplyLaterWillReturnDataForWriteRequestsWhenTheAssignedRequestKeyIsRepliedTo
)
{
	std::promise<uint64_t> callbackPromise{};
	auto callbackFuture = callbackPromise.get_future();
	std::vector<uint8_t> anyRequest{0x01, 0xAB, 0xC1};
	middleware::Reply anyReply{};
	SetupTestReply(anyReply);

	auto onRead = [](auto, auto const& request)
	{
		return ::middleware::Reply{};
	};
	auto onWrite = [&callbackPromise](auto key, auto const& request)
	{
		callbackPromise.set_value(key);
		return std::nullopt;
	};

	auto spObjectUnderTest = std::make_unique<::middleware::dds::ServerRequestHandler>(onRead, onWrite);

	::cat::middleware::dds::TransferData request{};
	request.protobuf(anyRequest);
	auto writeFuture = std::async(
	    &::middleware::dds::ServerRequestHandler::Write,
	    spObjectUnderTest.get(),
	    std::ref(*m_spMockRequestInfo),
	    request
	);

	WaitAndAssertReady(callbackFuture, n_ServerRequestInterval);
	spObjectUnderTest->Reply(callbackFuture.get(), anyReply);

	WaitAndAssertReady(writeFuture, n_ServerRequestInterval);
	auto repliedTransferData = writeFuture.get();
	EXPECT_THAT(repliedTransferData.protobuf(), Eq(anyReply.serialized_protobuf));
}
TEST_F(
    MiddlewareFrameworkServerRequestHandlerTest,
    ThrowRequestExceptionWithServerRequestSizeErrorWhenDeferredResponseIsGreaterThan4MB
)
{
	std::promise<uint64_t> callbackPromise{};
	auto callbackFuture = callbackPromise.get_future();
	std::vector<uint8_t> anyRequest{0x01, 0xAB, 0xC1};
	middleware::Reply oversizedReply{};
	oversizedReply.serialized_protobuf.resize(::middleware::max::n_MaxPayloadByteCount + 1);

	auto onRead = [&callbackPromise](auto key, auto const& request)
	{
		callbackPromise.set_value(key);
		return std::nullopt;
	};
	auto onWrite = [](auto, auto const& request)
	{
		return ::middleware::Reply{};
	};

	auto spObjectUnderTest = std::make_unique<::middleware::dds::ServerRequestHandler>(onRead, onWrite);

	::cat::middleware::dds::TransferData request{};
	request.protobuf(anyRequest);
	auto readFuture = std::async(
	    &::middleware::dds::ServerRequestHandler::Read, spObjectUnderTest.get(), std::ref(*m_spMockRequestInfo), request
	);

	WaitAndAssertReady(callbackFuture, n_ServerRequestInterval);
	spObjectUnderTest->Reply(callbackFuture.get(), oversizedReply);

	WaitAndAssertReady(readFuture, n_ServerRequestInterval);
	try
	{
		(void)readFuture.get();
		FAIL();
	}
	catch(::cat::middleware::dds::RequestException const& exception)
	{
		EXPECT_THAT(
		    exception.status_code(),
		    Eq(static_cast<uint32_t>(::middleware::StatusCode::STATUS_CODE_SERVER_REQUEST_SIZE_ERROR))
		);
		EXPECT_THAT(exception.request().protobuf(), Eq(anyRequest));
	}
	catch(...)
	{
		FAIL();
	}
}
