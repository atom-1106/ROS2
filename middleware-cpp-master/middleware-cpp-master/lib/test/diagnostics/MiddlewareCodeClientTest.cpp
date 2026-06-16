// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareCodeClientTest.cpp
// Description: Unit Tests for services/lib/diagnostics/types/CodeClient.cpp

#include "CodeClient.cpp"
#include "mockMiddlewareClient.h"
#include "mockMiddlewareReader.h"

#include <gmock/gmock.h>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest.h>

using namespace ::testing;
namespace
{
using DataCallback                  = MockFunction<void(::cat::middleware::diagnostics::Codes const&)>;
using AdditionalInformationCallback = MockFunction<void(::cat::middleware::diagnostics::AdditionalInformation&)>;
using DisconnectCallback            = MockFunction<void()>;
static constexpr uint32_t expectedCodeKey{7357};
}  // namespace

class MiddlewareCodeClientTest : public ::testing::Test
{
protected:
	MiddlewareCodeClientTest()
	    : m_spMockMiddlewareClient(std::make_unique<NiceMock<mock::MiddlewareClient>>()),
	      m_spMockMiddlewareReader(std::make_unique<NiceMock<mock::MiddlewareReader>>()) {};

	std::unique_ptr<NiceMock<mock::MiddlewareClient>> m_spMockMiddlewareClient;
	std::unique_ptr<NiceMock<mock::MiddlewareReader>> m_spMockMiddlewareReader;

	MockFunction<void(::cat::middleware::diagnostics::Codes const&)> m_mockOnSubscription;
	MockFunction<void()> m_mockOnDisconnect;
};

TEST_F(MiddlewareCodeClientTest, DeathTestConstructorPassedNullptrForReader)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(
	    std::make_unique<::middleware::diagnostics::CodeClient>(nullptr, std::move(m_spMockMiddlewareClient)),
	    "Assertion .* failed"
	);
}
TEST_F(MiddlewareCodeClientTest, DeathTestConstructorPassedNullptrForClient)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(
	    std::make_unique<::middleware::diagnostics::CodeClient>(std::move(m_spMockMiddlewareReader), nullptr),
	    "Assertion .* failed"
	);
}
TEST_F(MiddlewareCodeClientTest, EmptyCodeGroupsWillImmediatelyCallUserCallback)
{
	EXPECT_CALL(*m_spMockMiddlewareClient, Read(_, _)).Times(0);
	auto spUnderTest = std::make_unique<::middleware::diagnostics::CodeClient>(
	    std::move(m_spMockMiddlewareReader), std::move(m_spMockMiddlewareClient)
	);
	AdditionalInformationCallback mockOnComplete;
	EXPECT_CALL(mockOnComplete, Call(_));
	std::vector<::middleware::diagnostics::AdditionalInformationGroup> groups{};
	spUnderTest->RequestAdditionalInformation(expectedCodeKey, groups, mockOnComplete.AsStdFunction());
}
TEST_F(MiddlewareCodeClientTest, EmptyCodeGroupsWillStillCallUserCallbackEvenWithoutReplySuccess)
{
	EXPECT_CALL(*m_spMockMiddlewareClient, Read(_, _))
	    .WillOnce(Invoke(
	        [](auto const& req, auto&& callback)
	        {
		        ::middleware::Reply reply{};
		        reply.status.status_code = ::middleware::StatusCode::STATUS_CODE_SERVER_INTERNAL_ERROR;
		        callback(reply);
	        }
	    ));
	auto spUnderTest = std::make_unique<::middleware::diagnostics::CodeClient>(
	    std::move(m_spMockMiddlewareReader), std::move(m_spMockMiddlewareClient)
	);
	AdditionalInformationCallback mockOnComplete;
	EXPECT_CALL(mockOnComplete, Call(_));
	std::vector<::middleware::diagnostics::AdditionalInformationGroup> groups{::middleware::diagnostics::Group2};

	spUnderTest->RequestAdditionalInformation(expectedCodeKey, groups, mockOnComplete.AsStdFunction());
}
TEST_F(MiddlewareCodeClientTest, AvailableCodeGroup2WillCallUserCallbackEvenWithCodes)
{
	EXPECT_CALL(*m_spMockMiddlewareClient, Read(_, _))
	    .WillOnce(Invoke(
	        [](auto const& req, auto&& callback)
	        {
		        ::cat::middleware::diagnostics::AdditionalInformation codes{};
		        codes.set_code_key(expectedCodeKey);
		        codes.mutable_group_2()->set_occurrence_count(1);

		        auto serialized = codes.SerializeAsString();
		        ::middleware::Reply reply{};
		        reply.serialized_protobuf = std::vector<uint8_t>{std::cbegin(serialized), std::cend(serialized)};
		        reply.status.status_code  = ::middleware::StatusCode::STATUS_CODE_SUCCESS;
		        callback(reply);
	        }
	    ));
	auto spUnderTest = std::make_unique<::middleware::diagnostics::CodeClient>(
	    std::move(m_spMockMiddlewareReader), std::move(m_spMockMiddlewareClient)
	);
	AdditionalInformationCallback mockOnComplete;
	EXPECT_CALL(mockOnComplete, Call(_))
	    .WillOnce(Invoke(
	        [](auto const& data)
	        {
		        EXPECT_THAT(data.code_key(), Eq(expectedCodeKey));
		        ASSERT_TRUE(data.has_group_2());
		        EXPECT_THAT(data.group_2().occurrence_count(), Eq(1));
	        }
	    ));
	std::vector<::middleware::diagnostics::AdditionalInformationGroup> groups{::middleware::diagnostics::Group2};

	spUnderTest->RequestAdditionalInformation(expectedCodeKey, groups, mockOnComplete.AsStdFunction());
}
