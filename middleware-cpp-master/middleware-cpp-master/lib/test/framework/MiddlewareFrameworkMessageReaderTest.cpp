// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareFrameworkMessageReaderTest.cpp
// Description: Unit Tests for MessageReader class

#include "MessageReader.cpp"
#include "TestUtilities.h"
#include "mockMiddlewareDataReaderStub.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;

class MiddlewareFrameworkMessageReaderTest : public ::testing::Test
{
protected:
	MiddlewareFrameworkMessageReaderTest()
	    : m_spObjUnderTest{std::make_unique<::middleware::dds::MessageReader>(
	          "MyTopic",
	          m_mockOnSubscriptionCallback.AsStdFunction(),
	          nullptr
	      )}
	{
	}
	virtual ~MiddlewareFrameworkMessageReaderTest() = default;
	void CallSetDependency()
	{
		m_spObjUnderTest->SetDependency(m_spMockDataReaderStub.TakeUniquePtr());
	}
	void ConfigureTakeNextSampleExpectations(std::vector<std::vector<uint8_t>> const& dataToReturn)
	{
		auto&& expectation = EXPECT_CALL(*m_spMockDataReaderStub, TakeNextSample(_, _)).Times(dataToReturn.size() + 1);

		for(auto const& data : dataToReturn)
		{
			expectation.WillOnce(Invoke(
			    [data](auto* msg, auto* info)
			    {
				    info->valid_data                               = true;
				    ((::cat::middleware::Message*)msg)->protobuf() = data;
				    return ::eprosima::fastdds::dds::RETCODE_OK;
			    }
			));
		}

		expectation.WillRepeatedly(Return(::eprosima::fastdds::dds::RETCODE_NO_DATA));
	}

	MockFunction<void(std::vector<uint8_t> const&)> m_mockOnSubscriptionCallback;
	MockFunction<void()> m_mockOnDisconnectCallback;
	::middleware::testable::DefaultTestUniquePtr<NiceMock<mock::DataReaderStub>> m_spMockDataReaderStub{};
	std::unique_ptr<::middleware::dds::MessageReader> m_spObjUnderTest;
};

TEST_F(MiddlewareFrameworkMessageReaderTest, DeathTestConstructorAssertsWhenSubscriptionCallbackIsNullptr)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(
	    std::make_unique<::middleware::dds::MessageReader>(
	        "MyTopic", nullptr, m_mockOnDisconnectCallback.AsStdFunction()
	    ),
	    "Assertion .* failed"
	);
}
TEST_F(MiddlewareFrameworkMessageReaderTest, DeathTestSetDependencyAssertsWhenArgumentIsNullptr)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(m_spObjUnderTest->SetDependency(nullptr), "Assertion .* failed");
}
TEST_F(MiddlewareFrameworkMessageReaderTest, DeathTestSetDependencyAssertsWhenCalledMoreThanOnceWithNonNullptr)
{
	CallSetDependency();
	m_spMockDataReaderStub.Remake();

	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(CallSetDependency(), "Assertion .* failed");
}
TEST_F(MiddlewareFrameworkMessageReaderTest, ConstructionSucceedsWhenTopicStringIsEmpty)
{
	m_spObjUnderTest = std::make_unique<::middleware::dds::MessageReader>(
	    "", m_mockOnSubscriptionCallback.AsStdFunction(), m_mockOnDisconnectCallback.AsStdFunction()
	);
	EXPECT_TRUE(m_spObjUnderTest);
}
TEST_F(MiddlewareFrameworkMessageReaderTest, ConstructionSucceedsWhenDisconnectCallbackIsNullptr)
{
	m_spObjUnderTest = std::make_unique<::middleware::dds::MessageReader>(
	    "MyTopic", m_mockOnSubscriptionCallback.AsStdFunction(), nullptr
	);
	EXPECT_TRUE(m_spObjUnderTest);
}
TEST_F(
    MiddlewareFrameworkMessageReaderTest,
    ConnectedToPublisherReturnsFalseWhenOnSubscriptionMatchedHasNeverBeenCalled
)
{
	EXPECT_FALSE(m_spObjUnderTest->ConnectedToPublisher());
}
TEST_F(MiddlewareFrameworkMessageReaderTest, ConnectedToPublisherReturnsTrueWhenOnSubscriptionMatchedIsPassedTrue)
{
	m_spObjUnderTest->OnSubscriptionMatched(true);
	EXPECT_TRUE(m_spObjUnderTest->ConnectedToPublisher());
}
TEST_F(MiddlewareFrameworkMessageReaderTest, ConnectedToPublisherReturnsFalseWhenOnSubscriptionMatchedIsPassedFalse)
{
	m_spObjUnderTest->OnSubscriptionMatched(false);
	EXPECT_FALSE(m_spObjUnderTest->ConnectedToPublisher());
}

TEST_F(MiddlewareFrameworkMessageReaderTest, OnDisconnectOnlyCalledWhenChangingFromMatchedTrueToFalse)
{

	m_spObjUnderTest = std::make_unique<::middleware::dds::MessageReader>(
	    "MyTopic", m_mockOnSubscriptionCallback.AsStdFunction(), m_mockOnDisconnectCallback.AsStdFunction()
	);
	m_spObjUnderTest->OnSubscriptionMatched(false);
	m_spObjUnderTest->OnSubscriptionMatched(false);
	m_spObjUnderTest->OnSubscriptionMatched(false);
	m_spObjUnderTest->OnSubscriptionMatched(false);
	m_spObjUnderTest->OnSubscriptionMatched(true);
	m_spObjUnderTest->OnSubscriptionMatched(true);
	m_spObjUnderTest->OnSubscriptionMatched(true);

	EXPECT_CALL(m_mockOnDisconnectCallback, Call());
	m_spObjUnderTest->OnSubscriptionMatched(false);  // Disconnect
	m_spObjUnderTest->OnSubscriptionMatched(false);
	EXPECT_FALSE(m_spObjUnderTest->ConnectedToPublisher());
}
TEST_F(MiddlewareFrameworkMessageReaderTest, HavingNoDisconnectCallbackWillNotCrashWhenDisconnectHappens)
{
	m_spObjUnderTest->OnSubscriptionMatched(true);
	m_spObjUnderTest->OnSubscriptionMatched(false);
	EXPECT_FALSE(m_spObjUnderTest->ConnectedToPublisher());
}
TEST_F(MiddlewareFrameworkMessageReaderTest, SubscriptionNotCalledWhenOnDataAvailableReturnCodeIsNotOk)
{
	EXPECT_CALL(m_mockOnSubscriptionCallback, Call(_)).Times(0);
	EXPECT_CALL(*m_spMockDataReaderStub, TakeNextSample(_, _))
	    .WillOnce(Return(::eprosima::fastdds::dds::RETCODE_ERROR));

	CallSetDependency();
	m_spObjUnderTest->OnDataAvailable();
}
TEST_F(
    MiddlewareFrameworkMessageReaderTest,
    SubscriptionNotCalledWhenOnDataAvailableReturnCodeIsOkButMessageDataIsInvalid
)
{
	EXPECT_CALL(m_mockOnSubscriptionCallback, Call(_)).Times(0);
	EXPECT_CALL(*m_spMockDataReaderStub, TakeNextSample(_, _))
	    .WillOnce(Invoke(
	        [](auto* msg, auto* info)
	        {
		        info->valid_data = false;
		        return ::eprosima::fastdds::dds::RETCODE_OK;
	        }
	    ))
	    .WillRepeatedly(Return(::eprosima::fastdds::dds::RETCODE_NO_DATA));

	CallSetDependency();
	m_spObjUnderTest->OnDataAvailable();
}
TEST_F(MiddlewareFrameworkMessageReaderTest, StopsCallingTakeNextSampleWhenOnDataAvailableReturnCodeIsNotOKorNO_DATA)
{
	auto const anyOtherRetcode = ::eprosima::fastdds::dds::RETCODE_ERROR;
	EXPECT_CALL(*m_spMockDataReaderStub, TakeNextSample(_, _)).WillRepeatedly(Return(anyOtherRetcode));
	EXPECT_CALL(m_mockOnSubscriptionCallback, Call(_)).Times(0);

	CallSetDependency();
	m_spObjUnderTest->OnDataAvailable();
}
TEST_F(MiddlewareFrameworkMessageReaderTest, SubscriptionCalledWhenOnDataAvailableReturnCodeIsOkAndMessageDataIsValid)
{
	CallSetDependency();

	std::vector<uint8_t> const expectedData{1, 2, 3};
	ConfigureTakeNextSampleExpectations({expectedData});

	EXPECT_CALL(m_mockOnSubscriptionCallback, Call(expectedData));
	m_spObjUnderTest->OnDataAvailable();
}
TEST_F(MiddlewareFrameworkMessageReaderTest, AllSamplesAreProcessed)
{
	CallSetDependency();

	std::vector<uint8_t> const expectedData1{1, 2};
	std::vector<uint8_t> const expectedData2{3, 4};
	ConfigureTakeNextSampleExpectations({expectedData1, expectedData2});

	{
		InSequence seq{};
		EXPECT_CALL(m_mockOnSubscriptionCallback, Call(expectedData1));
		EXPECT_CALL(m_mockOnSubscriptionCallback, Call(expectedData2));
	}

	m_spObjUnderTest->OnDataAvailable();
}
