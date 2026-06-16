// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareFrameworkMessageParticipantTest.cpp
// Description: Unit Tests for MessageParticipant class

#include "MessageParticipant.cpp"
#include "mockMiddlewareDomainParticipantStub.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;
namespace
{
static constexpr uint32_t n_AnyTestLocatorPort = 10000;
}  // namespace

class MiddlewareFrameworkMessageParticipantTest : public ::testing::Test
{
protected:
	MiddlewareFrameworkMessageParticipantTest()
	    : m_spMockDomainParticipantStub{std::make_unique<NiceMock<mock::DomainParticipantStub>>()},
	      m_pMockDomainParticipantStub{m_spMockDomainParticipantStub.get()},
	      m_spObjUnderTest{
	          std::make_unique<::middleware::dds::MessageParticipant>(std::move(m_spMockDomainParticipantStub))
	      }
	{
	}
	virtual ~MiddlewareFrameworkMessageParticipantTest() = default;

	std::unique_ptr<NiceMock<mock::DomainParticipantStub>> m_spMockDomainParticipantStub;
	NiceMock<mock::DomainParticipantStub>* m_pMockDomainParticipantStub;
	std::unique_ptr<::middleware::dds::MessageParticipant> m_spObjUnderTest;
};

TEST_F(MiddlewareFrameworkMessageParticipantTest, DeathTestConstructorAssertsWhenStubIsNullptr)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(std::make_unique<::middleware::dds::MessageParticipant>(nullptr), "Assertion .* failed");
}
TEST_F(MiddlewareFrameworkMessageParticipantTest, TheHardCodedDefinedQosPolicyIsPassedToStubWhenCreateWriterIsCalled)
{
	std::string expected_topic = "SomeTopic";

	// Note: This is the expected QOS for Standard Middleware Publishers. Any adjustments will trigger updates to this test.
	auto expected_qos                = ::eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT;
	expected_qos.publish_mode().kind = ::eprosima::fastdds::dds::SYNCHRONOUS_PUBLISH_MODE;
	expected_qos.endpoint().history_memory_policy =
	    ::eprosima::fastdds::rtps::MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
	expected_qos.history().kind     = ::eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
	expected_qos.durability().kind  = ::eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
	expected_qos.reliability().kind = ::eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
	expected_qos.reliability().max_blocking_time.seconds             = 0;
	expected_qos.reliability().max_blocking_time.nanosec             = 100000000;  // 100ms
	expected_qos.history().depth                                     = 1;
	expected_qos.resource_limits().max_samples                       = 1;
	expected_qos.resource_limits().max_instances                     = 1;
	expected_qos.resource_limits().max_samples_per_instance          = 1;
	expected_qos.reliable_writer_qos().disable_positive_acks.enabled = true;
	expected_qos.reliable_writer_qos().disable_heartbeat_piggyback   = false;
	expected_qos.data_sharing().off();

	// Note: The mocks return is irrelevant, nullptr is fine.
	EXPECT_CALL(*m_pMockDomainParticipantStub, CreateWriter(expected_topic, expected_qos)).WillOnce(Return(nullptr));

	m_spObjUnderTest->CreateWriter(expected_topic);
}
TEST_F(MiddlewareFrameworkMessageParticipantTest, TheHardCodedDefinedQosPolicyIsPassedToStubWhenCreateReaderIsCalled)
{
	std::string expected_topic = "SomeTopic";

	// Note: This is the expected QOS for Standard Middleware Subscribers. Any adjustments will trigger updates to this test.
	auto expected_qos = ::eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT;
	expected_qos.endpoint().history_memory_policy =
	    ::eprosima::fastdds::rtps::MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
	expected_qos.history().kind     = ::eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
	expected_qos.durability().kind  = ::eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
	expected_qos.reliability().kind = ::eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
	expected_qos.reliability().max_blocking_time.seconds    = 0;
	expected_qos.reliability().max_blocking_time.nanosec    = 100000000;  // 100ms
	expected_qos.history().depth                            = 1;
	expected_qos.resource_limits().max_samples              = 1;
	expected_qos.resource_limits().max_instances            = 1;
	expected_qos.resource_limits().max_samples_per_instance = 1;
	::eprosima::fastdds::rtps::Locator_t multicast_locator;
	::eprosima::fastdds::rtps::IPLocator::setIPv4(multicast_locator, 239, 'B', 'O', 'G');
	multicast_locator.port = n_AnyTestLocatorPort;
	expected_qos.endpoint().multicast_locator_list.push_back(multicast_locator);
	expected_qos.reliable_reader_qos().disable_positive_acks.enabled = true;
	expected_qos.data_sharing().off();

	EXPECT_CALL(*m_pMockDomainParticipantStub, MulticastListeningLocatorPort()).WillOnce(Return(n_AnyTestLocatorPort));
	// Note: The mocks return is irrelevant, nullptr is fine.
	EXPECT_CALL(*m_pMockDomainParticipantStub, CreateReader(expected_topic, expected_qos, _, _))
	    .WillOnce(Return(nullptr));

	m_spObjUnderTest->CreateReader(expected_topic, nullptr, nullptr);
}
TEST_F(MiddlewareFrameworkMessageParticipantTest, CallbacksAreCapturedByStubWhenCreateReaderIsCalled)
{
	std::string expected_topic = "SomeTopic";
	std::vector<uint8_t> some_bytes{0xAA, 0x1D, 0x31};

	MockFunction<void(std::vector<uint8_t> const&)> mockOnSubscriptionCallback;
	MockFunction<void()> mockOnDisconnectCallback;
	EXPECT_CALL(mockOnSubscriptionCallback, Call(some_bytes));
	EXPECT_CALL(mockOnDisconnectCallback, Call());

	// Note: The mocks return is irrelevant, nullptr is fine.
	EXPECT_CALL(*m_pMockDomainParticipantStub, CreateReader(expected_topic, _, _, _))
	    .WillOnce(Invoke(
	        [&some_bytes](auto const& topic, auto const& qos, auto&& subscription, auto&& disconnect)
	        {
		        subscription(some_bytes);
		        disconnect();
		        return nullptr;
	        }
	    ));

	m_spObjUnderTest->CreateReader(
	    expected_topic, mockOnSubscriptionCallback.AsStdFunction(), mockOnDisconnectCallback.AsStdFunction()
	);
}
