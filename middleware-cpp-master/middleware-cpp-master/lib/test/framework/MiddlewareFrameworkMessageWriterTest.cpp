// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareFrameworkMessageWriterTest.cpp
// Description: Unit Tests for MessageWriter class

#include "MessageWriter.cpp"
#include "mockMiddlewareDataWriterStub.h"

#include <gmock/gmock.h>
#include <google/protobuf/any.pb.h>
#include <gtest/gtest.h>

using namespace ::testing;
namespace
{
// Note: Discovered via debug prints
static constexpr uint32_t n_ProtobufAnyBaseAllocatedMetaDataSize = 5;
}  // namespace

class MiddlewareFrameworkMessageWriterTest : public ::testing::Test
{
protected:
	MiddlewareFrameworkMessageWriterTest()
	    : m_spMockDataWriterStub{std::make_unique<NiceMock<mock::DataWriterStub>>()},
	      m_spObjUnderTest{std::make_unique<::middleware::dds::MessageWriter>("MyTopic")}
	{
	}
	virtual ~MiddlewareFrameworkMessageWriterTest() = default;

	std::unique_ptr<NiceMock<mock::DataWriterStub>> m_spMockDataWriterStub;
	std::unique_ptr<::middleware::dds::MessageWriter> m_spObjUnderTest;
};

TEST_F(MiddlewareFrameworkMessageWriterTest, DeathTestSetDependencyAssertsWhenArgumentIsNullptr)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(m_spObjUnderTest->SetDependency(nullptr), "Assertion .* failed");
}
TEST_F(MiddlewareFrameworkMessageWriterTest, DeathTestSetDependencyAssertsWhenCalledMoreThanOnceWithNonNullptr)
{
	auto spMockDataWriterStub = std::make_unique<NiceMock<mock::DataWriterStub>>();
	m_spObjUnderTest->SetDependency(std::move(m_spMockDataWriterStub));

	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(m_spObjUnderTest->SetDependency(std::move(spMockDataWriterStub)), "Assertion .* failed");
}
TEST_F(MiddlewareFrameworkMessageWriterTest, ConnectedToSubscriberReturnsTrueWhenOnPublicationMatchedIsPassedTrue)
{
	m_spObjUnderTest->OnPublicationMatched(true);
	EXPECT_TRUE(m_spObjUnderTest->ConnectedToSubscriber());
}
TEST_F(MiddlewareFrameworkMessageWriterTest, ConnectedToSubscriberReturnsFalseWhenOnPublicationMatchedIsPassedFalse)
{
	m_spObjUnderTest->OnPublicationMatched(false);
	EXPECT_FALSE(m_spObjUnderTest->ConnectedToSubscriber());
}
TEST_F(
    MiddlewareFrameworkMessageWriterTest,
    ConnectedToSubscriberReturnsCorrectStatusesWhenOnPublicationMatchedIsPassedTrueAndFalseInDifferentSequences
)
{
	m_spObjUnderTest->OnPublicationMatched(false);
	m_spObjUnderTest->OnPublicationMatched(false);
	EXPECT_FALSE(m_spObjUnderTest->ConnectedToSubscriber());
	m_spObjUnderTest->OnPublicationMatched(true);
	EXPECT_TRUE(m_spObjUnderTest->ConnectedToSubscriber());
	m_spObjUnderTest->OnPublicationMatched(false);
	m_spObjUnderTest->OnPublicationMatched(true);
	m_spObjUnderTest->OnPublicationMatched(true);
	EXPECT_TRUE(m_spObjUnderTest->ConnectedToSubscriber());
	m_spObjUnderTest->OnPublicationMatched(false);
	EXPECT_FALSE(m_spObjUnderTest->ConnectedToSubscriber());
}
TEST_F(MiddlewareFrameworkMessageWriterTest, BytesPublishReturnsFalseWithNoCallToStubWhenContentIsGreaterThan4MB)
{
	EXPECT_CALL(*m_spMockDataWriterStub, Write(_)).Times(0);
	m_spObjUnderTest->SetDependency(std::move(m_spMockDataWriterStub));

	std::vector<uint8_t> tooLargeData(::middleware::max::n_MaxPayloadByteCount + 1, 0xA);
	EXPECT_FALSE(m_spObjUnderTest->Publish(tooLargeData));
}
TEST_F(MiddlewareFrameworkMessageWriterTest, BytesPublishReturnsFalseWhenReturnCodeIsNotOk)
{
	EXPECT_CALL(*m_spMockDataWriterStub, Write(_)).WillOnce(Return(::eprosima::fastdds::dds::RETCODE_ERROR));
	m_spObjUnderTest->SetDependency(std::move(m_spMockDataWriterStub));

	std::vector<uint8_t> someData{0x01, 0x02, 0x03};
	EXPECT_FALSE(m_spObjUnderTest->Publish(someData));
}
TEST_F(MiddlewareFrameworkMessageWriterTest, BytesPublishReturnsTrueWhenReturnCodeIsOk)
{
	EXPECT_CALL(*m_spMockDataWriterStub, Write(_)).WillOnce(Return(::eprosima::fastdds::dds::RETCODE_OK));
	m_spObjUnderTest->SetDependency(std::move(m_spMockDataWriterStub));

	std::vector<uint8_t> someData{0x01, 0x02, 0x03};
	EXPECT_TRUE(m_spObjUnderTest->Publish(someData));
}
TEST_F(MiddlewareFrameworkMessageWriterTest, BytesPublishReturnsTrueWhenReturnCodeIsOkWith4MBContent)
{
	EXPECT_CALL(*m_spMockDataWriterStub, Write(_)).WillOnce(Return(::eprosima::fastdds::dds::RETCODE_OK));
	m_spObjUnderTest->SetDependency(std::move(m_spMockDataWriterStub));

	std::vector<uint8_t> atBoundData(::middleware::max::n_MaxPayloadByteCount, 0xA);
	EXPECT_TRUE(m_spObjUnderTest->Publish(atBoundData));
}
TEST_F(MiddlewareFrameworkMessageWriterTest, BytesPublishReturnsTrueWhenReturnCodeIsOkWithLessThan4MBContent)
{
	EXPECT_CALL(*m_spMockDataWriterStub, Write(_)).WillOnce(Return(::eprosima::fastdds::dds::RETCODE_OK));
	m_spObjUnderTest->SetDependency(std::move(m_spMockDataWriterStub));

	std::vector<uint8_t> belowBoundData(::middleware::max::n_MaxPayloadByteCount - 1, 0xA);
	EXPECT_TRUE(m_spObjUnderTest->Publish(belowBoundData));
}
TEST_F(MiddlewareFrameworkMessageWriterTest, ProtobufPublishReturnsFalseWithNoCallToStubWhenContentIsGreaterThan4MB)
{

	EXPECT_CALL(*m_spMockDataWriterStub, Write(_)).Times(0);
	m_spObjUnderTest->SetDependency(std::move(m_spMockDataWriterStub));

	::google::protobuf::Any tooLargeData{};
	tooLargeData.set_value(std::string(::middleware::max::n_MaxPayloadByteCount + 1, 'A'));
	EXPECT_FALSE(m_spObjUnderTest->Publish(tooLargeData));
}
TEST_F(MiddlewareFrameworkMessageWriterTest, ProtobufPublishReturnsFalseWhenReturnCodeIsNotOk)
{
	EXPECT_CALL(*m_spMockDataWriterStub, Write(_)).WillOnce(Return(::eprosima::fastdds::dds::RETCODE_ERROR));
	m_spObjUnderTest->SetDependency(std::move(m_spMockDataWriterStub));

	::google::protobuf::Any someData{};
	someData.set_value("some value");
	EXPECT_FALSE(m_spObjUnderTest->Publish(someData));
}
TEST_F(MiddlewareFrameworkMessageWriterTest, ProtobufPublishReturnsTrueWhenReturnCodeIsOk)
{
	EXPECT_CALL(*m_spMockDataWriterStub, Write(_)).WillOnce(Return(::eprosima::fastdds::dds::RETCODE_OK));
	m_spObjUnderTest->SetDependency(std::move(m_spMockDataWriterStub));

	::google::protobuf::Any someData{};
	someData.set_value("some value");
	EXPECT_TRUE(m_spObjUnderTest->Publish(someData));
}
TEST_F(MiddlewareFrameworkMessageWriterTest, ProtobufPublishReturnsTrueWhenReturnCodeIsOkWith4MBContent)
{
	EXPECT_CALL(*m_spMockDataWriterStub, Write(_)).WillOnce(Return(::eprosima::fastdds::dds::RETCODE_OK));
	m_spObjUnderTest->SetDependency(std::move(m_spMockDataWriterStub));

	::google::protobuf::Any atBoundData{};
	// offset due to protobuf metadata
	atBoundData.set_value(
	    std::string(::middleware::max::n_MaxPayloadByteCount - n_ProtobufAnyBaseAllocatedMetaDataSize, 'A')
	);
	EXPECT_TRUE(m_spObjUnderTest->Publish(atBoundData));
}
TEST_F(MiddlewareFrameworkMessageWriterTest, ProtobufPublishReturnsTrueWhenReturnCodeIsOkWithLessThan4MBContent)
{
	EXPECT_CALL(*m_spMockDataWriterStub, Write(_)).WillOnce(Return(::eprosima::fastdds::dds::RETCODE_OK));
	m_spObjUnderTest->SetDependency(std::move(m_spMockDataWriterStub));

	::google::protobuf::Any belowBoundData{};
	// offset due to protobuf metadata
	belowBoundData.set_value(
	    std::string(::middleware::max::n_MaxPayloadByteCount - n_ProtobufAnyBaseAllocatedMetaDataSize - 1, 'A')
	);
	EXPECT_TRUE(m_spObjUnderTest->Publish(belowBoundData));
}
