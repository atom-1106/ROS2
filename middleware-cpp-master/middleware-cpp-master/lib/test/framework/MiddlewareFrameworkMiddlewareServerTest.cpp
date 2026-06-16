// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareFrameworkMiddlewareServerTest.cpp
// Description: Unit Tests for MiddlewareServer class

#include "MiddlewareServer.cpp"
#include "TestUtilities.h"
#include "mockMiddlewareServerRpcStub.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;

namespace
{
static constexpr char n_TestServerName[] = "TestServer";
}  // namespace

class MiddlewareFrameworkMiddlewareServerTest : public ::testing::Test
{
protected:
	MiddlewareFrameworkMiddlewareServerTest()
	    : m_spMockServerRpcStub{std::make_unique<NiceMock<::mock::ServerRpcStub>>()}

	{
	}
	virtual ~MiddlewareFrameworkMiddlewareServerTest() = default;
	std::unique_ptr<NiceMock<::mock::ServerRpcStub>> m_spMockServerRpcStub;
};

TEST_F(MiddlewareFrameworkMiddlewareServerTest, DeathTestConstructorPassedNullptrForRpcServerStub)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(std::make_unique<::middleware::dds::Server>(n_TestServerName, nullptr), "Assertion .* failed");
}
TEST_F(MiddlewareFrameworkMiddlewareServerTest, ServerTearsDownWithoutIssuesWhenRunIsNeverCalled)
{
	auto spServer = std::make_unique<::middleware::dds::Server>(n_TestServerName, std::move(m_spMockServerRpcStub));
	EXPECT_TRUE(spServer);
}
TEST_F(MiddlewareFrameworkMiddlewareServerTest, ServerStubRunIsCalledWhenSeverCallsRun)
{
	EXPECT_CALL(*m_spMockServerRpcStub, Run());
	auto spServer = std::make_unique<::middleware::dds::Server>(n_TestServerName, std::move(m_spMockServerRpcStub));
	EXPECT_TRUE(spServer);
	spServer->Run();
}
TEST_F(MiddlewareFrameworkMiddlewareServerTest, ServerStubStopIsCalledTwiceWhenSeverDirectlyCallsStop)
{
	EXPECT_CALL(*m_spMockServerRpcStub, Run());
	// Called again due to destruction
	EXPECT_CALL(*m_spMockServerRpcStub, Stop()).Times(2);

	auto spServer = std::make_unique<::middleware::dds::Server>(n_TestServerName, std::move(m_spMockServerRpcStub));
	EXPECT_TRUE(spServer);
	spServer->Run();
	spServer->Stop();
}
TEST_F(MiddlewareFrameworkMiddlewareServerTest, ServerStubStopIsCalledOnceWhenSeverDoesNotDirectlyCallStop)
{
	EXPECT_CALL(*m_spMockServerRpcStub, Run());
	EXPECT_CALL(*m_spMockServerRpcStub, Stop());

	auto spServer = std::make_unique<::middleware::dds::Server>(n_TestServerName, std::move(m_spMockServerRpcStub));
	EXPECT_TRUE(spServer);
	spServer->Run();
	spServer.reset();
}
TEST_F(MiddlewareFrameworkMiddlewareServerTest, ServerStubReplyIsCalledOnceWhenSeverCallsReply)
{
	uint_least16_t requestKey = 123;
	middleware::Reply anyReply{};
	anyReply.serialized_protobuf   = {0x01, 0xAB, 0xC1};
	anyReply.status.status_code    = ::middleware::StatusCode::STATUS_CODE_SUCCESS;
	anyReply.status.status_message = "Test Success";

	EXPECT_CALL(*m_spMockServerRpcStub, Reply(requestKey, _))
	    .WillOnce(Invoke(
	        [&anyReply](auto const&, auto const& reply)
	        {
		        EXPECT_THAT(reply.serialized_protobuf, Eq(anyReply.serialized_protobuf));
		        EXPECT_THAT(reply.status.status_code, Eq(anyReply.status.status_code));
		        EXPECT_THAT(reply.status.status_message, Eq(anyReply.status.status_message));
	        }
	    ));

	auto spServer = std::make_unique<::middleware::dds::Server>(n_TestServerName, std::move(m_spMockServerRpcStub));
	ASSERT_TRUE(spServer);
	spServer->Reply(requestKey, anyReply);
}
