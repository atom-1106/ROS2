// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareCodeServerTest.cpp
// Description: Unit Tests for services/lib/diagnostics/types/CodeServer.cpp

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "CodeServer.cpp"
#include "mockMiddlewareServer.h"
#include "mockMiddlewareWriter.h"

#include <google/protobuf/util/message_differencer.h>

using namespace ::testing;

class MiddlewareCodeServerTest : public ::testing::Test
{
protected:
	MiddlewareCodeServerTest()
	    : m_spMockMiddlewareServer(std::make_unique<NiceMock<mock::MiddlewareServer>>()),
	      m_spMockMiddlewareWriter(std::make_unique<NiceMock<mock::MiddlewareWriter>>()) {

	      };
	virtual ~MiddlewareCodeServerTest()
	{
	}
	std::unique_ptr<NiceMock<::mock::MiddlewareServer>> m_spMockMiddlewareServer;
	std::unique_ptr<NiceMock<::mock::MiddlewareWriter>> m_spMockMiddlewareWriter;

	::cat::middleware::diagnostics::Codes m_someCodes{};
};
MATCHER_P(AreProtosEqual, expected_message, testing::PrintToString(expected_message))
{
	::google::protobuf::util::MessageDifferencer differ;
	return differ.Compare(expected_message, arg);
}
TEST_F(MiddlewareCodeServerTest, DeathTestConstructorPassedNullptrForPublishWriter)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(
	    std::make_unique<::middleware::diagnostics::CodeServer>(
	        m_someCodes, nullptr, std::move(m_spMockMiddlewareServer)
	    ),
	    "Assertion .* failed"
	);
}
TEST_F(MiddlewareCodeServerTest, DeathTestConstructorRetrievedServerNullptr)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(
	    std::make_unique<::middleware::diagnostics::CodeServer>(
	        m_someCodes, std::move(m_spMockMiddlewareWriter), nullptr
	    ),
	    "Assertion .* failed"
	);
}
TEST_F(MiddlewareCodeServerTest, DiagnosticServerPublishesInitialCodesOnCreation)
{
	EXPECT_CALL(*m_spMockMiddlewareWriter, Publish(An<::google::protobuf::Message const&>())).WillOnce(Return(true));
	auto spObjectUnderTest = std::make_unique<::middleware::diagnostics::CodeServer>(
	    m_someCodes, std::move(m_spMockMiddlewareWriter), std::move(m_spMockMiddlewareServer)
	);
	EXPECT_TRUE(spObjectUnderTest);
}
TEST_F(MiddlewareCodeServerTest, DiagnosticServerPublishesInitialCodesOnCreationButFailureDoesNotCauseCrash)
{
	EXPECT_CALL(*m_spMockMiddlewareWriter, Publish(An<::google::protobuf::Message const&>())).WillOnce(Return(false));
	auto spObjectUnderTest = std::make_unique<::middleware::diagnostics::CodeServer>(
	    m_someCodes, std::move(m_spMockMiddlewareWriter), std::move(m_spMockMiddlewareServer)
	);
	EXPECT_TRUE(spObjectUnderTest);
}
TEST_F(MiddlewareCodeServerTest, DiagnosticServerCallsServerRunOnCreation)
{
	EXPECT_CALL(*m_spMockMiddlewareServer, Run());
	auto spObjectUnderTest = std::make_unique<::middleware::diagnostics::CodeServer>(
	    m_someCodes, std::move(m_spMockMiddlewareWriter), std::move(m_spMockMiddlewareServer)
	);
	EXPECT_TRUE(spObjectUnderTest);
}
TEST_F(MiddlewareCodeServerTest, DiagnosticServerCallsServerStopOnDestruction)
{
	EXPECT_CALL(*m_spMockMiddlewareServer, Stop());
	auto spObjectUnderTest = std::make_unique<::middleware::diagnostics::CodeServer>(
	    m_someCodes, std::move(m_spMockMiddlewareWriter), std::move(m_spMockMiddlewareServer)
	);
	EXPECT_TRUE(spObjectUnderTest);
	spObjectUnderTest.reset();
}
TEST_F(MiddlewareCodeServerTest, DiagnosticServerPublishesCodesWhenPublishIsCalled)
{
	::cat::middleware::diagnostics::Codes someNewCodes{};
	auto code         = someNewCodes.add_codes();
	auto j1939Address = code->mutable_network_address()->mutable_j1939_address();
	j1939Address->set_address(42);
	j1939Address->set_can_port(::cat::middleware::diagnostics::NetworkAddress::J1939Address::CAN_PORT_1);
	code->set_code_key(7357);

	// Once on creation, again by calling Publish
	EXPECT_CALL(*m_spMockMiddlewareWriter, Publish(An<::google::protobuf::Message const&>()))
	    .WillOnce(Invoke(
	        [this](auto const& codes)
	        {
		        EXPECT_THAT(codes, AreProtosEqual(m_someCodes));
		        return true;
	        }
	    ))
	    .WillOnce(Invoke(
	        [&someNewCodes](auto const& codes)
	        {
		        EXPECT_THAT(codes, AreProtosEqual(someNewCodes));
		        return true;
	        }
	    ));
	auto spObjectUnderTest = std::make_unique<::middleware::diagnostics::CodeServer>(
	    m_someCodes, std::move(m_spMockMiddlewareWriter), std::move(m_spMockMiddlewareServer)
	);
	EXPECT_TRUE(spObjectUnderTest);

	spObjectUnderTest->Publish(someNewCodes);
}
