// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: CommonFrameworkTest.cpp
// Description: Unit Tests for services/lib/framework/common/MiddlewareFrameworkDefines.h and
//              services/lib/framework/common/HermesLog.h.

#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <bit>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <future>
#include <memory>
#include <string>
#include <utility>
#include "DomainParticipantMessageStub.h"
#include "Finally.h"
#include "MessageParticipant.h"
#include "MiddlewareDefines.h"
#include "ObfuscatedKey.h"
#include "middleware_parameter.pb.h"

using namespace ::testing;
namespace
{
static constexpr uint32_t n_StatusCodeMin = 0;
static constexpr uint32_t n_StatusCodeMax = 0;

}  // namespace

class CommonFrameworkTest : public ::testing::Test
{
protected:
	CommonFrameworkTest()          = default;
	virtual ~CommonFrameworkTest() = default;
};

TEST_F(CommonFrameworkTest, StructuresOnCreationAreDefaultAssigned)
{
	::middleware::Request request{};
	EXPECT_THAT(request.serialized_protobuf, IsEmpty());

	::middleware::Reply reply{};
	EXPECT_THAT(reply.status.status_code, Eq(::middleware::StatusCode::STATUS_CODE_UNSPECIFIED));
	EXPECT_THAT(reply.status.status_message, IsEmpty());
	EXPECT_THAT(reply.serialized_protobuf, IsEmpty());

	::middleware::Status status{};
	EXPECT_THAT(status.status_code, Eq(::middleware::StatusCode::STATUS_CODE_UNSPECIFIED));
	EXPECT_THAT(status.status_message, IsEmpty());
}
TEST_F(CommonFrameworkTest, IndexShuffleLUTIsOneToOne)
{
	size_t const testSize = 1000;
	auto lut              = ::middleware::indexShuffleLUT<testSize>();
	std::sort(lut.begin(), lut.end());
	for(size_t i = 0; i < testSize; i++)
	{
		ASSERT_EQ(lut[i], i);
	}
}
TEST_F(CommonFrameworkTest, BytesObfuscatedKeyMaintainsNumberOfSetBits)
{
	uint64_t sourceKey = 0;
	ASSERT_EQ(::middleware::bytesObfuscatedKey(sourceKey), 0);
	for(size_t i = 0; i < 64; i++)
	{
		sourceKey |= (uint64_t)1 << i;
		ASSERT_EQ(std::popcount(::middleware::bytesObfuscatedKey(sourceKey)), std::popcount(sourceKey));
	}
}
TEST_F(CommonFrameworkTest, BitsObfuscatedKeyMaintainsNumberOfSetBits)
{
	uint64_t sourceKey = 0;
	ASSERT_EQ(::middleware::bitsObfuscatedKey(sourceKey), 0);
	for(size_t i = 0; i < 64; i++)
	{
		sourceKey |= (uint64_t)1 << i;
		ASSERT_EQ(std::popcount(::middleware::bitsObfuscatedKey(sourceKey)), std::popcount(sourceKey));
	}
}
TEST_F(CommonFrameworkTest, ShmCleanupDoesNotBreakLocalCommunication)
{
	// Uses the ShmCleanup application to attempt to clean shared memory mid-usage from a separate process
	// One pub two subs before cleaning
	// Checks that those two still work
	// Checks that adding another after the clean still is able to connect and get the latest value
	uint8_t expectedValue = 1;

	auto spTestSubscriber1 = std::make_unique<::middleware::dds::MessageParticipant>(
	    std::make_unique<::middleware::dds::DomainParticipantMessageStub>(0, "TestSubscriber1")
	);
	auto spTestSubscriber2 = std::make_unique<::middleware::dds::MessageParticipant>(
	    std::make_unique<::middleware::dds::DomainParticipantMessageStub>(0, "TestSubscriber2")
	);
	MockFunction<void(std::vector<uint8_t> const&)> mockOnSubscriptionCallback1;
	MockFunction<void(std::vector<uint8_t> const&)> mockOnSubscriptionCallback2;
	auto spReader1 = spTestSubscriber1->CreateReader("TestTopic", mockOnSubscriptionCallback1.AsStdFunction(), nullptr);
	auto spReader2 = spTestSubscriber2->CreateReader("TestTopic", mockOnSubscriptionCallback2.AsStdFunction(), nullptr);

	auto spTestPublisher = std::make_unique<::middleware::dds::MessageParticipant>(
	    std::make_unique<::middleware::dds::DomainParticipantMessageStub>(0, "TestPublisher")
	);
	auto spWriter = spTestPublisher->CreateWriter("TestTopic");

	auto const ExpectThatSubReceivedValue = [&expectedValue](auto& callback, auto& promise)
	{
		EXPECT_CALL(callback, Call(_))
		    .WillOnce(Invoke([&promise, &expectedValue](std::vector<uint8_t> const& value)
		                     { promise.set_value(value.size() == 1 && value[0] == expectedValue); }));
	};

	std::promise<bool> promise1;
	std::promise<bool> promise2;
	auto future1 = promise1.get_future();
	auto future2 = promise2.get_future();
	ExpectThatSubReceivedValue(mockOnSubscriptionCallback1, promise1);
	ExpectThatSubReceivedValue(mockOnSubscriptionCallback2, promise2);
	spWriter->Publish(std::vector<uint8_t>{expectedValue});
	ASSERT_TRUE(future1.get());
	ASSERT_TRUE(future2.get());

	std::string exe = std::filesystem::canonical("/proc/" + std::to_string(getpid()) + "/exe").string();
	exe.resize(exe.rfind("/", exe.rfind("/", std::string::npos) - 1));  // remove /test/executable
	exe = exe + "/apps/ShmCleanup";
	ASSERT_TRUE(std::filesystem::exists(exe));
	ASSERT_NE(std::system(NULL), 0);
	std::ignore = std::system(exe.c_str());

	auto spTestSubscriber3 = std::make_unique<::middleware::dds::MessageParticipant>(
	    std::make_unique<::middleware::dds::DomainParticipantMessageStub>(0, "TestSubscriber3")
	);
	MockFunction<void(std::vector<uint8_t> const&)> mockOnSubscriptionCallback3;
	auto spReader3 = spTestSubscriber3->CreateReader("TestTopic", mockOnSubscriptionCallback3.AsStdFunction(), nullptr);

	promise1 = {};
	promise2 = {};
	std::promise<bool> promise3;
	future1      = promise1.get_future();
	future2      = promise2.get_future();
	auto future3 = promise3.get_future();
	Mock::VerifyAndClearExpectations(&mockOnSubscriptionCallback1);
	Mock::VerifyAndClearExpectations(&mockOnSubscriptionCallback2);
	ExpectThatSubReceivedValue(mockOnSubscriptionCallback1, promise1);
	ExpectThatSubReceivedValue(mockOnSubscriptionCallback2, promise2);
	ExpectThatSubReceivedValue(mockOnSubscriptionCallback3, promise3);
	spWriter->Publish(std::vector<uint8_t>{expectedValue});
	ASSERT_EQ(future1.wait_for(std::chrono::milliseconds(1000)), std::future_status::ready);
	ASSERT_EQ(future2.wait_for(std::chrono::milliseconds(1000)), std::future_status::ready);
	ASSERT_EQ(future3.wait_for(std::chrono::milliseconds(1000)), std::future_status::ready);
	EXPECT_TRUE(future1.get());
	EXPECT_TRUE(future2.get());
	EXPECT_TRUE(future3.get());
}
TEST_F(CommonFrameworkTest, ShmCleanupCleansUpStaleSemaphores)
{
	char semName[] = "fastdds_port7661_mutex";
	sem_t* pSem    = sem_open(semName, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 0);
	if(pSem == SEM_FAILED)
	{
		pSem = sem_open(semName, O_EXCL, 0, 0);
	}
	ASSERT_NE(pSem, SEM_FAILED);
	sem_close(pSem);

	std::string exe = std::filesystem::canonical("/proc/" + std::to_string(getpid()) + "/exe").string();
	exe.resize(exe.rfind("/", exe.rfind("/", std::string::npos) - 1));  // remove /test/executable
	exe = exe + "/apps/ShmCleanup";
	ASSERT_TRUE(std::filesystem::exists(exe));
	ASSERT_NE(std::system(NULL), 0);
	std::ignore = std::system(exe.c_str());

	pSem = sem_open(semName, 0, 0, 0);
	EXPECT_EQ(pSem, SEM_FAILED);
}
TEST_F(CommonFrameworkTest, ShmCleanupCleansUpOtherEntries)
{
	char tmpDir[strlen("/tmp/MiddlewareTest_") + 6 + 1] = "/tmp/MiddlewareTest_XXXXXX";
	ASSERT_THAT(mkdtemp(&tmpDir[0]), ::testing::NotNull());
	auto cleanUp = Finally::Do([&tmpDir]() { std::filesystem::remove_all(tmpDir); });
	// taken from a system after killing an application
	std::vector<char const*> files{
	    "fastdds_026b18adb7506bb8",
	    "fastdds_026b18adb7506bb8_el",
	    "fastdds_d9e36d87517f35be",
	    "fastdds_d9e36d87517f35be_sl",
	    "fastdds_port7661",
	    "fastdds_port7661_el",
	    "fastdds_port7663",
	    "fastdds_port7663_el",
	    "fastdds_port7667",
	    // no lock file for 7667
	    "fastrtps_026b18adb7506bb8",
	    "fastrtps_026b18adb7506bb8_el",
	    "fastrtps_d9e36d87517f35be",
	    "fastrtps_d9e36d87517f35be_sl",
	    "fastrtps_port7661",
	    "fastrtps_port7661_el",
	    "fastrtps_port7663",
	    "fastrtps_port7663_el",
	    "fastrtps_port7667"
	    // no lock file for 7667
	};
	for(auto const& file : files)
	{
		int fd = creat((std::string{tmpDir} + std::string{file}).c_str(), O_RDONLY);
		ASSERT_GE(fd, 0);
		close(fd);
	}

	std::string exe = std::filesystem::canonical("/proc/" + std::to_string(getpid()) + "/exe").string();
	exe.resize(exe.rfind("/", exe.rfind("/", std::string::npos) - 1));  // remove /test/executable
	exe = exe + "/apps/ShmCleanup";
	ASSERT_TRUE(std::filesystem::exists(exe));
	ASSERT_NE(std::system(NULL), 0);
	std::ignore = std::system((exe + " " + std::string{tmpDir}).c_str());

	auto count = std::distance(std::filesystem::directory_iterator(tmpDir), std::filesystem::directory_iterator{});
	ASSERT_EQ(count, 0);
}
