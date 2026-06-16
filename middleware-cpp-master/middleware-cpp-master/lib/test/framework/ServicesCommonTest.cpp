// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ServicesCommonTest.cpp
// Description: Unit Tests for services/lib/common.

#include "FakeTestUtilities.h"
#include "MiddlewareDefines.h"
#include "SignalConnection.h"
#include "Signalling.h"
#include "SimpleTaskSignal.h"
#include "SystemUtilities.cpp"

#include <gmock/gmock.h>
#include <google/protobuf/any.pb.h>
#include <gtest/gtest.h>
#include <memory>

using namespace ::testing;
using namespace std::chrono_literals;
namespace
{
}  // namespace

class ServicesCommonTest : public ::testing::Test
{
protected:
	virtual ~ServicesCommonTest()
	{
		::middleware::log::Reset();
	}

public:
	std::stringstream m_stream{};
	NiceMock<MockFunction<void()>> m_callback{};
};
TEST_F(ServicesCommonTest, TimeNowReturnsMyCurrentTimeInMilliseconds)
{
	auto const time               = FakeClock::get();
	auto const timestampUnderTest = ::middleware::common::TimeNowAsMs();
	auto const expected           = std::chrono::duration_cast<std::chrono::milliseconds>(
                              ::middleware::testable::system_clock::now().time_since_epoch()
    )
	                          .count();
	EXPECT_THAT(timestampUnderTest, Eq(expected));
}
TEST_F(ServicesCommonTest, TimeNowAsFormatReturnsCurrentTimeAsFormattedString)
{
	auto const time               = FakeClock::get();
	auto const timestampUnderTest = ::middleware::common::TimeNowAsFormat();
	auto const now                = ::middleware::testable::system_clock::now();
	auto const time_t_now         = std::chrono::system_clock::to_time_t(now);
	// Convert to tm struct for local time
	std::tm tm_now{};
	localtime_r(&time_t_now, &tm_now);
	// Format date and time
	std::ostringstream oss{};
	oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
	// Add milliseconds
	auto const milliseconds =
	    std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
	oss << ':' << std::setfill('0') << std::setw(3) << milliseconds;
	EXPECT_THAT(timestampUnderTest, Eq(oss.str()));
}
TEST_F(ServicesCommonTest, NoIssuesWhenCallingBoostSignalCallback)
{
	EXPECT_CALL(m_callback, Call());
	::middleware::Signalling<void()> m_someEvent{};
	auto const connection = m_someEvent.Register(m_callback.AsStdFunction());
	EXPECT_TRUE(connection);
	m_someEvent.FireEvent();
}
TEST_F(ServicesCommonTest, ReturnFalseWhenTaskReachesTimeout)
{
	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_FOUR);

	auto const time     = FakeClock::get();
	auto const callback = [&time]()
	{
		(*time)->flush_time(100);  // SimpleTaskSignal has to start waiting on promise
		(*time)->advance_ms(::middleware::timeouts_ms::frameworkReadWrite + 1);
	};
	::middleware::SimpleTaskSignal task(std::move(callback));
	auto const result = task.Wait(1s);

	EXPECT_FALSE(result);
	EXPECT_THAT(m_stream.str(), HasSubstr("Timeout occurred. Terminating the thread."));
}
TEST_F(ServicesCommonTest, ReturnTrueWhenTaskReachesCompletionBeforeTimeout)
{
	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_FOUR);

	auto const time     = FakeClock::get();
	auto const callback = [&time]()
	{
		(*time)->advance_ms(4999);  // default timeout is 5s
		(*time)->flush_time();
	};
	::middleware::SimpleTaskSignal task(std::move(callback));
	auto const result = task.Wait();

	EXPECT_TRUE(result);
	EXPECT_THAT(m_stream.str(), IsEmpty());
}
TEST_F(ServicesCommonTest, LogWarningSkipInitWhenLoggerInitializeIsCalledMoreThanOnce)
{
	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_TWO);
	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_THREE);
	::middleware::log::Info("Screaming into the void");
	EXPECT_THAT(m_stream.str(), HasSubstr("Middleware Logger already initialized!"));
	EXPECT_THAT(m_stream.str(), Not(HasSubstr("Screaming into the void")));
}
TEST_F(ServicesCommonTest, DoNotLogAnythingWhenInitializeHasNotBeenCalled)
{
	::middleware::log::Warning("I am %u\n", 2);
	::middleware::log::Info("I am %u\n", 3);
	::middleware::log::Debug("I am %u\n", 4);

	EXPECT_THAT(m_stream.str(), IsEmpty());
}
TEST_F(ServicesCommonTest, LogErrorWhenCalled)
{
	std::string log = "Some Error!!";
	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_FOUR);
	EXPECT_THROW(::middleware::log::Error(log), std::runtime_error);
	EXPECT_THAT(m_stream.str(), HasSubstr(log));
}
TEST_F(ServicesCommonTest, LogWarningWhenCalled)
{
	std::string log = "Some Warning!!";
	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_FOUR);
	::middleware::log::Warning(log);
	EXPECT_THAT(m_stream.str(), HasSubstr(log));
}
TEST_F(ServicesCommonTest, LogInfoWhenCalled)
{
	std::string log = "Some Info!!";
	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_FOUR);
	::middleware::log::Info(log);
	EXPECT_THAT(m_stream.str(), HasSubstr(log));
}
TEST_F(ServicesCommonTest, LogDebugWhenCalled)
{
	std::string log = "Some Debug Info!!";
	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_FOUR);
	::middleware::log::Debug(log);
	EXPECT_THAT(m_stream.str(), HasSubstr(log));
}
TEST_F(ServicesCommonTest, LogErrorWithArgsWhenCalled)
{
	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_FOUR);
	EXPECT_THROW(::middleware::log::Error("The value is %u", 1), std::runtime_error);
	EXPECT_THAT(m_stream.str(), HasSubstr("The value is 1"));
}
TEST_F(ServicesCommonTest, LogWarningWithArgsWhenCalled)
{
	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_FOUR);
	::middleware::log::Warning("The value is %u", 200);
	EXPECT_THAT(m_stream.str(), HasSubstr("The value is 200"));
}
TEST_F(ServicesCommonTest, LogInfoWithArgsWhenCalled)
{
	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_FOUR);
	::middleware::log::Info("The value is %u", 43);
	EXPECT_THAT(m_stream.str(), HasSubstr("The value is 43"));
}
TEST_F(ServicesCommonTest, LogDebugWithArgsWhenCalled)
{
	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_FOUR);
	::middleware::log::Debug("The value is %u", 92);
	EXPECT_THAT(m_stream.str(), HasSubstr("The value is 92"));
}
TEST_F(ServicesCommonTest, DoNotLogDebugWithLevelOtherThanFourWhenCalled)
{
	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_ONE);
	::middleware::log::Debug("LEVEL_ONE");
	::middleware::log::Reset();

	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_TWO);
	::middleware::log::Debug("LEVEL_TWO");
	::middleware::log::Reset();

	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_THREE);
	::middleware::log::Debug("LEVEL_THREE");
	::middleware::log::Reset();
	EXPECT_THAT(m_stream.str(), IsEmpty());
}
TEST_F(ServicesCommonTest, DoNotLogInfoWithLevelOtherThanThreeWhenCalled)
{
	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_ONE);
	::middleware::log::Info("LEVEL_ONE");
	::middleware::log::Reset();

	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_TWO);
	::middleware::log::Info("LEVEL_TWO");
	::middleware::log::Reset();

	EXPECT_THAT(m_stream.str(), IsEmpty());
}
TEST_F(ServicesCommonTest, DoNotLogWarningWithLevelOtherThanTwoWhenCalled)
{
	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_ONE);
	::middleware::log::Warning("LEVEL_ONE");
	EXPECT_THAT(m_stream.str(), IsEmpty());
}
TEST_F(ServicesCommonTest, LogErrorWithLevelTwoWhenCalled)
{
	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_TWO);
	EXPECT_THROW(::middleware::log::Error("My Log Message"), std::runtime_error);
	EXPECT_THAT(m_stream.str(), HasSubstr("My Log Message"));
}
TEST_F(ServicesCommonTest, LogErrorWithLevelThreeWhenCalled)
{
	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_THREE);
	EXPECT_THROW(::middleware::log::Error("My Log Message"), std::runtime_error);
	EXPECT_THAT(m_stream.str(), HasSubstr("My Log Message"));
}
TEST_F(ServicesCommonTest, LogErrorWithLevelFourWhenCalled)
{
	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_FOUR);
	EXPECT_THROW(::middleware::log::Error("My Log Message"), std::runtime_error);
	EXPECT_THAT(m_stream.str(), HasSubstr("My Log Message"));
}
TEST_F(ServicesCommonTest, LogWarningWithLevelThreeWhenCalled)
{
	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_THREE);
	::middleware::log::Warning("My Log Message");
	EXPECT_THAT(m_stream.str(), HasSubstr("My Log Message"));
}
TEST_F(ServicesCommonTest, LogWarningWithLevelFourWhenCalled)
{
	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_FOUR);
	::middleware::log::Warning("My Log Message");
	EXPECT_THAT(m_stream.str(), HasSubstr("My Log Message"));
}
TEST_F(ServicesCommonTest, LogInfoWithLevelFourWhenCalled)
{
	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_FOUR);
	::middleware::log::Info("My Log Message");
	EXPECT_THAT(m_stream.str(), HasSubstr("My Log Message"));
}
TEST_F(ServicesCommonTest, LoggerCanBeResetToDifferentOutputStreamWhenResetIsCalled)
{
	::middleware::log::Initialize(m_stream, middleware::log::LogLevel::LEVEL_FOUR);
	EXPECT_THROW(::middleware::log::Error("Logging with stringstream"), std::runtime_error);
	EXPECT_THAT(m_stream.str(), HasSubstr("Logging with stringstream"));
	::middleware::log::Reset();
	::middleware::log::Initialize(std::cout, middleware::log::LogLevel::LEVEL_FOUR);
	EXPECT_THROW(::middleware::log::Error("Logging with cout"), std::runtime_error);
}
TEST_F(ServicesCommonTest, SerializeToVectorReturnsUsableVectorWhenCalled)
{
	std::string data{"My test data"};
	::google::protobuf::Any original{};
	original.set_value(data);
	auto serialized = ::middleware::common::SerializeToVector(original);
	::google::protobuf::Any underTest{};
	EXPECT_TRUE(underTest.ParseFromArray(serialized.data(), serialized.size()));
	EXPECT_THAT(underTest.value(), Eq(data));
}
TEST_F(ServicesCommonTest, SerializeToVectorReturnsUsableVectorWhenCalledWithEmptyMessage)
{
	::google::protobuf::Any original{};
	auto serialized = ::middleware::common::SerializeToVector(original);

	EXPECT_THAT(serialized, IsEmpty());
	::google::protobuf::Any underTest{};
	EXPECT_TRUE(underTest.ParseFromArray(serialized.data(), serialized.size()));
	EXPECT_THAT(underTest.value(), IsEmpty());
}
TEST_F(ServicesCommonTest, SerializeToVectorAssignsUsableVectorWhenCalled)
{
	std::string data{"My test data"};
	::google::protobuf::Any original{};
	original.set_value(data);

	std::vector<uint8_t> serialized{};
	::middleware::common::SerializeToVector(original, serialized);
	::google::protobuf::Any underTest{};
	EXPECT_TRUE(underTest.ParseFromArray(serialized.data(), serialized.size()));
	EXPECT_THAT(underTest.value(), Eq(data));
}
TEST_F(ServicesCommonTest, SerializeToVectorAssignsUsableVectorWhenCalledWithEmptyMessage)
{
	::google::protobuf::Any original{};

	std::vector<uint8_t> serialized{};
	::middleware::common::SerializeToVector(original, serialized);

	EXPECT_THAT(serialized, IsEmpty());
	::google::protobuf::Any underTest{};
	EXPECT_TRUE(underTest.ParseFromArray(serialized.data(), serialized.size()));
	EXPECT_THAT(underTest.value(), IsEmpty());
}
