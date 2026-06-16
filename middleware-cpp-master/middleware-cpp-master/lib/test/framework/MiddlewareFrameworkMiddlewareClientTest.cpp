// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareFrameworkMiddlewareClientTest.cpp
// Description: Unit Tests for MiddlewareClient class

#include "MiddlewareClient.cpp"
#include "mockMiddlewareClientRpcStub.h"
#include "mockMiddlewareRequests.h"
#include "mockMiddlewareThreadPool.h"

#include "TestUtilities.h"

#include <gmock/gmock.h>
#include <google/protobuf/any.pb.h>
#include <gtest/gtest.h>

using namespace ::testing;
namespace
{
static constexpr char n_TestClientName[] = "TestClient";
static std::chrono::milliseconds const n_SomeDuration{100};
}  // namespace

class MiddlewareFrameworkMiddlewareClientTest : public ::testing::Test
{
protected:
	MiddlewareFrameworkMiddlewareClientTest()
	    : m_spMockClientRpcStub{std::make_unique<NiceMock<::mock::ClientRpcStub>>()},
	      m_spMockThreadPool{std::make_unique<NiceMock<::mock::ThreadPool>>()},
	      m_spMockRequests{std::make_unique<NiceMock<::mock::Requests>>()}
	{
		m_someRequest.set_value("Hello World");
	}
	virtual ~MiddlewareFrameworkMiddlewareClientTest() = default;
	std::unique_ptr<NiceMock<::mock::ClientRpcStub>> m_spMockClientRpcStub;
	std::unique_ptr<NiceMock<::mock::ThreadPool>> m_spMockThreadPool;
	std::shared_ptr<NiceMock<::mock::Requests>> m_spMockRequests;
	MockFunction<void(::middleware::Reply const&)> m_mockOnCompleteCallback;

	::google::protobuf::Any m_someRequest{};
	std::promise<::cat::middleware::dds::TransferData> m_promise;
};

TEST_F(MiddlewareFrameworkMiddlewareClientTest, DeathTestConstructorPassedNullptrForRpcClientStub)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(
	    std::make_unique<::middleware::dds::Client>(
	        n_TestClientName, n_SomeDuration, nullptr, std::move(m_spMockThreadPool)
	    ),
	    "Assertion .* failed"
	);
}
TEST_F(MiddlewareFrameworkMiddlewareClientTest, DeathTestConstructorPassedNullptrForThreadPool)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(
	    std::make_unique<::middleware::dds::Client>(
	        n_TestClientName, n_SomeDuration, std::move(m_spMockClientRpcStub), nullptr
	    ),
	    "Assertion .* failed"
	);
}

namespace
{
struct ReadWriteTestData
{
	std::string name{};
	static std::string TestName(testing::TestParamInfo<ReadWriteTestData> const& data)
	{
		return data.param.name;
	}
};
ReadWriteTestData const n_TestDataForReadWrite[] = {{"READ"}, {"WRITE"}};
}  // namespace

class ReadWriteRequests : public MiddlewareFrameworkMiddlewareClientTest, public WithParamInterface<ReadWriteTestData>
{
};
INSTANTIATE_TEST_SUITE_P(
    MiddlewareFrameworkMiddlewareClientTest,
    ReadWriteRequests,
    ValuesIn(n_TestDataForReadWrite),
    ReadWriteTestData::TestName
);

TEST_P(ReadWriteRequests, ClientRequestTimeoutWhenRpcFutureIsNotReady)
{
	EXPECT_CALL(*m_spMockClientRpcStub, GetInstance())
	    .WillOnce(Invoke(
	        [this]()
	        {
		        if(GetParam().name == "READ")
		        {
			        EXPECT_CALL(*m_spMockRequests, Read(_))
			            .WillOnce(Invoke([this](auto const& request) { return m_promise.get_future(); }));
		        }
		        else
		        {
			        EXPECT_CALL(*m_spMockRequests, Write(_))
			            .WillOnce(Invoke([this](auto const& request) { return m_promise.get_future(); }));
		        }
		        return std::weak_ptr<::cat::middleware::dds::Requests>{m_spMockRequests};
	        }
	    ));

	EXPECT_CALL(*m_spMockThreadPool, Add(_)).WillOnce(Invoke([](auto&& task) { task(); }));
	auto spClient = std::make_unique<::middleware::dds::Client>(
	    n_TestClientName, n_SomeDuration, std::move(m_spMockClientRpcStub), std::move(m_spMockThreadPool)
	);

	EXPECT_CALL(m_mockOnCompleteCallback, Call(_))
	    .WillOnce(Invoke(
	        [this](auto const& reply)
	        {
		        EXPECT_THAT(reply.status.status_code, Eq(::middleware::StatusCode::STATUS_CODE_CLIENT_TIMEOUT_ERROR));
		        m_promise.set_value(::cat::middleware::dds::TransferData{});
	        }
	    ));

	if(GetParam().name == "READ")
	{
		spClient->Read(m_someRequest, m_mockOnCompleteCallback.AsStdFunction());
	}
	else
	{
		spClient->Write(m_someRequest, m_mockOnCompleteCallback.AsStdFunction());
	}
}
TEST_P(ReadWriteRequests, ClientRequestInternalInitErrorWhenStubIsDestroyed)
{
	EXPECT_CALL(*m_spMockClientRpcStub, GetInstance())
	    .WillOnce(Invoke([this]() { return std::weak_ptr<::cat::middleware::dds::Requests>(); }));

	EXPECT_CALL(*m_spMockThreadPool, Add(_)).WillOnce(Invoke([](auto&& task) { task(); }));
	auto spClient = std::make_unique<::middleware::dds::Client>(
	    n_TestClientName, n_SomeDuration, std::move(m_spMockClientRpcStub), std::move(m_spMockThreadPool)
	);

	EXPECT_CALL(m_mockOnCompleteCallback, Call(_))
	    .WillOnce(Invoke(
	        [](auto const& reply)
	        { EXPECT_THAT(reply.status.status_code, Eq(::middleware::StatusCode::STATUS_CODE_CLIENT_INIT_ERROR)); }
	    ));

	if(GetParam().name == "READ")
	{
		spClient->Read(m_someRequest, m_mockOnCompleteCallback.AsStdFunction());
	}
	else
	{
		spClient->Write(m_someRequest, m_mockOnCompleteCallback.AsStdFunction());
	}
}
TEST_P(ReadWriteRequests, ClientRequestCompletesWithReplyWhenReadDataIsAvailable)
{
	std::vector<uint8_t> expectedReply{0x00, 0xAB, 0x1C};
	EXPECT_CALL(*m_spMockClientRpcStub, GetInstance())
	    .WillOnce(Invoke(
	        [&expectedReply, this]()
	        {
		        if(GetParam().name == "READ")
		        {
			        EXPECT_CALL(*m_spMockRequests, Read(_))
			            .WillOnce(Invoke(
			                [&expectedReply, this](auto const& request)
			                {
				                auto future = m_promise.get_future();
				                ::cat::middleware::dds::TransferData reply{};
				                reply.protobuf(expectedReply);
				                m_promise.set_value(reply);
				                return future;
			                }
			            ));
		        }
		        else
		        {
			        EXPECT_CALL(*m_spMockRequests, Write(_))
			            .WillOnce(Invoke(
			                [&expectedReply, this](auto const& request)
			                {
				                auto future = m_promise.get_future();
				                ::cat::middleware::dds::TransferData reply{};
				                reply.protobuf(expectedReply);
				                m_promise.set_value(reply);
				                return future;
			                }
			            ));
		        }
		        return std::weak_ptr<::cat::middleware::dds::Requests>{m_spMockRequests};
	        }
	    ));

	EXPECT_CALL(*m_spMockThreadPool, Add(_)).WillOnce(Invoke([](auto&& task) { task(); }));
	auto spClient = std::make_unique<::middleware::dds::Client>(
	    n_TestClientName, n_SomeDuration, std::move(m_spMockClientRpcStub), std::move(m_spMockThreadPool)
	);

	EXPECT_CALL(m_mockOnCompleteCallback, Call(_))
	    .WillOnce(Invoke(
	        [&expectedReply](auto const& reply)
	        {
		        EXPECT_THAT(reply.status.status_code, Eq(::middleware::StatusCode::STATUS_CODE_SUCCESS));
		        EXPECT_THAT(reply.serialized_protobuf, Eq(expectedReply));
	        }
	    ));

	if(GetParam().name == "READ")
	{
		spClient->Read(m_someRequest, m_mockOnCompleteCallback.AsStdFunction());
	}
	else
	{
		spClient->Write(m_someRequest, m_mockOnCompleteCallback.AsStdFunction());
	}
}
TEST_P(ReadWriteRequests, ClientRequestReturnsSizeErrorWhenPayloadExceedsMaxByteCount)
{
	// Create a request that exceeds the max payload size
	auto oversizedRequest = m_someRequest;
	std::string largeValue(::middleware::max::n_MaxPayloadByteCount + 1000, 'A');
	oversizedRequest.set_value(largeValue);

	// Verify that ThreadPool::Add is NOT called (callback should be invoked immediately)
	EXPECT_CALL(*m_spMockThreadPool, Add(_)).Times(0);

	auto spClient = std::make_unique<::middleware::dds::Client>(
	    n_TestClientName, n_SomeDuration, std::move(m_spMockClientRpcStub), std::move(m_spMockThreadPool)
	);

	EXPECT_CALL(m_mockOnCompleteCallback, Call(_))
	    .WillOnce(Invoke(
	        [&largeValue](auto const& reply)
	        {
		        EXPECT_THAT(
		            reply.status.status_code, Eq(::middleware::StatusCode::STATUS_CODE_CLIENT_REQUEST_SIZE_ERROR)
		        );
		        EXPECT_THAT(
		            reply.status.status_message, HasSubstr(std::to_string(::middleware::max::n_MaxPayloadByteCount))
		        );
		        EXPECT_GT(reply.serialized_protobuf.size(), ::middleware::max::n_MaxPayloadByteCount);
	        }
	    ));

	if(GetParam().name == "READ")
	{
		spClient->Read(oversizedRequest, m_mockOnCompleteCallback.AsStdFunction());
	}
	else
	{
		spClient->Write(oversizedRequest, m_mockOnCompleteCallback.AsStdFunction());
	}
}
TEST_P(ReadWriteRequests, ClientRequestReturnsSizeErrorWithExactlyAtBoundary)
{
	// Create a request that is exactly at the max payload size (should succeed)
	auto boundaryRequest = m_someRequest;
	// n_MaxPayloadByteCount includes protobuf overhead, so create a value that fits within it
	std::string boundaryValue(::middleware::max::n_MaxPayloadByteCount - 100, 'B');
	boundaryRequest.set_value(boundaryValue);

	EXPECT_CALL(*m_spMockClientRpcStub, GetInstance())
	    .WillOnce(Invoke(
	        [this]()
	        {
		        if(GetParam().name == "READ")
		        {
			        EXPECT_CALL(*m_spMockRequests, Read(_))
			            .WillOnce(Invoke([this](auto const& request) { return m_promise.get_future(); }));
		        }
		        else
		        {
			        EXPECT_CALL(*m_spMockRequests, Write(_))
			            .WillOnce(Invoke([this](auto const& request) { return m_promise.get_future(); }));
		        }
		        return std::weak_ptr<::cat::middleware::dds::Requests>{m_spMockRequests};
	        }
	    ));

	EXPECT_CALL(*m_spMockThreadPool, Add(_)).WillOnce(Invoke([](auto&& task) { task(); }));
	auto spClient = std::make_unique<::middleware::dds::Client>(
	    n_TestClientName, n_SomeDuration, std::move(m_spMockClientRpcStub), std::move(m_spMockThreadPool)
	);

	EXPECT_CALL(m_mockOnCompleteCallback, Call(_))
	    .WillOnce(Invoke(
	        [](auto const& reply)
	        { EXPECT_THAT(reply.status.status_code, Eq(::middleware::StatusCode::STATUS_CODE_CLIENT_TIMEOUT_ERROR)); }
	    ));

	if(GetParam().name == "READ")
	{
		spClient->Read(boundaryRequest, m_mockOnCompleteCallback.AsStdFunction());
	}
	else
	{
		spClient->Write(boundaryRequest, m_mockOnCompleteCallback.AsStdFunction());
	}
}
TEST_P(ReadWriteRequests, ClientRequestSizeErrorDoesNotInvokeDdsRpc)
{
	// Create a request that exceeds the max payload size
	auto oversizedRequest = m_someRequest;
	std::string largeValue(::middleware::max::n_MaxPayloadByteCount + 500, 'C');
	oversizedRequest.set_value(largeValue);

	// Verify that GetInstance (and therefore DDS RPC) is NOT called
	EXPECT_CALL(*m_spMockClientRpcStub, GetInstance()).Times(0);
	EXPECT_CALL(*m_spMockThreadPool, Add(_)).Times(0);

	auto spClient = std::make_unique<::middleware::dds::Client>(
	    n_TestClientName, n_SomeDuration, std::move(m_spMockClientRpcStub), std::move(m_spMockThreadPool)
	);

	bool callbackInvoked = false;
	auto onComplete      = [&callbackInvoked](auto const& reply)
	{
		callbackInvoked = true;
		EXPECT_THAT(reply.status.status_code, Eq(::middleware::StatusCode::STATUS_CODE_CLIENT_REQUEST_SIZE_ERROR));
	};

	if(GetParam().name == "READ")
	{
		spClient->Read(oversizedRequest, onComplete);
	}
	else
	{
		spClient->Write(oversizedRequest, onComplete);
	}

	EXPECT_TRUE(callbackInvoked);
}
// Write
