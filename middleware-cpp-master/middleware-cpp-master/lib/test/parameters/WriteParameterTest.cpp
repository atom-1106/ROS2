// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: WriteParameterTest.cpp
// Description: Unit Tests for WriteParameter class

#include "WriteParameter.cpp"
#include "mockMiddlewareClient.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;
namespace
{
static constexpr char n_SomeParameterName[] = "SomeParameter";
static constexpr auto n_SomeUnit            = ::middleware::parameter::Unit::degree;
static ::middleware::parameter::Parameter TestParameter(
    ::cat::middleware::parameter::Quality quality = ::cat::middleware::parameter::QUALITY_GOOD
)
{
	::middleware::parameter::Parameter parameter{};
	parameter.mutable_info()->set_identity(n_SomeParameterName);
	parameter.mutable_info()->set_unit(static_cast<uint32_t>(n_SomeUnit));
	parameter.mutable_data()->mutable_value()->set_string_value("Test");
	parameter.mutable_data()->set_quality(quality);
	return parameter;
}
using MockWriteOnCompleteFunction = NiceMock<
    MockFunction<void(::middleware::parameter::IWrite::WriteStatus const&, ::middleware::parameter::Parameter const&)>>;
}  // namespace

class WriteParameterTest : public ::testing::Test
{
protected:
	WriteParameterTest()
	    : m_spMockMiddlewareClient{std::make_shared<NiceMock<mock::MiddlewareClient>>()},
	      m_mockOnCompleteCallback{},
	      m_spObjUnderTest{
	          std::make_unique<::middleware::parameter::WriteParameter>(m_parameterConfig, m_spMockMiddlewareClient)
	      }
	{
	}
	virtual ~WriteParameterTest() = default;

	::middleware::parameter::client::WriteParameter m_parameterConfig{n_SomeParameterName, n_SomeUnit};
	std::shared_ptr<NiceMock<mock::MiddlewareClient>> m_spMockMiddlewareClient;
	MockWriteOnCompleteFunction m_mockOnCompleteCallback;
	std::unique_ptr<::middleware::parameter::WriteParameter> m_spObjUnderTest;
};

TEST_F(WriteParameterTest, DeathTestConstructorPassedNullptrMiddlewareClient)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(
	    std::make_unique<::middleware::parameter::WriteParameter>(m_parameterConfig, nullptr), "Assertion .* failed"
	);
}
TEST_F(WriteParameterTest, DeathTestWritePassedNullCallback)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(m_spObjUnderTest->Write(TestParameter().data(), nullptr), "Assertion .* failed");
}

TEST_F(WriteParameterTest, WriteCallsCallbackWithFailureWhenStatusCodeNotSuccess)
{
	EXPECT_CALL(*m_spMockMiddlewareClient, Write(_, _))
	    .WillOnce(Invoke(
	        [](auto const& protobuf, auto&& callback)
	        {
		        ::middleware::Reply reply{};
		        reply.status.status_code    = ::middleware::StatusCode::STATUS_CODE_CLIENT_TIMEOUT_ERROR;
		        reply.status.status_message = "some error";
		        callback(reply);
	        }
	    ));

	EXPECT_CALL(m_mockOnCompleteCallback, Call(_, _))
	    .WillOnce(Invoke([](auto const& status, auto const& parameter)
	                     { EXPECT_THAT(status.status, Eq(::middleware::parameter::Status::FAILURE)); }));

	m_spObjUnderTest->Write(TestParameter().data(), m_mockOnCompleteCallback.AsStdFunction());
}

TEST_F(WriteParameterTest, WriteCallsCallbackWithSuccessWhenQualityGood)
{
	EXPECT_CALL(*m_spMockMiddlewareClient, Write(_, _))
	    .WillOnce(Invoke(
	        [](auto const& protobuf, auto&& callback)
	        {
		        ::middleware::parameter::Parameter parameter{};
		        parameter.CopyFrom(protobuf);
		        EXPECT_THAT(parameter.info().identity(), Eq(n_SomeParameterName));
		        EXPECT_THAT(parameter.info().unit(), Eq(static_cast<uint32_t>(n_SomeUnit)));
		        parameter.mutable_data()->set_quality(::cat::middleware::parameter::QUALITY_GOOD);

		        ::middleware::Reply reply{};
		        reply.status.status_code  = ::middleware::StatusCode::STATUS_CODE_SUCCESS;
		        reply.serialized_protobuf = ::middleware::common::SerializeToVector(parameter.data());
		        callback(reply);
	        }
	    ));

	EXPECT_CALL(m_mockOnCompleteCallback, Call(_, _))
	    .WillOnce(Invoke([](auto const& status, auto const& parameter)
	                     { EXPECT_THAT(status.status, Eq(::middleware::parameter::Status::SUCCESS)); }));

	m_spObjUnderTest->Write(TestParameter().data(), m_mockOnCompleteCallback.AsStdFunction());
}

TEST_F(WriteParameterTest, WriteCallsCallbackWithFailureWhenQualityNotGood)
{
	EXPECT_CALL(*m_spMockMiddlewareClient, Write(_, _))
	    .WillOnce(Invoke(
	        [](auto const& protobuf, auto&& callback)
	        {
		        ::middleware::parameter::Parameter parameter{};
		        parameter.CopyFrom(protobuf);
		        EXPECT_THAT(parameter.info().identity(), Eq(n_SomeParameterName));
		        EXPECT_THAT(parameter.info().unit(), Eq(static_cast<uint32_t>(n_SomeUnit)));
		        parameter.mutable_data()->set_quality(::cat::middleware::parameter::QUALITY_BAD);

		        ::middleware::Reply reply{};
		        reply.status.status_code  = ::middleware::StatusCode::STATUS_CODE_SUCCESS;
		        reply.serialized_protobuf = ::middleware::common::SerializeToVector(parameter.data());
		        callback(reply);
	        }
	    ));

	EXPECT_CALL(m_mockOnCompleteCallback, Call(_, _))
	    .WillOnce(Invoke([](auto const& status, auto const& parameter)
	                     { EXPECT_THAT(status.status, Eq(::middleware::parameter::Status::FAILURE)); }));

	m_spObjUnderTest->Write(TestParameter().data(), m_mockOnCompleteCallback.AsStdFunction());
}

TEST_F(WriteParameterTest, WriteCallsCallbackWithFailureWhenReplyCannotBeParsed)
{
	EXPECT_CALL(*m_spMockMiddlewareClient, Write(_, _))
	    .WillOnce(Invoke(
	        [](auto const& protobuf, auto&& callback)
	        {
		        ::middleware::Reply reply{};
		        reply.status.status_code  = ::middleware::StatusCode::STATUS_CODE_SUCCESS;
		        reply.serialized_protobuf = {0xFF, 0xFF, 0xFF, 0xFF};
		        callback(reply);
	        }
	    ));

	EXPECT_CALL(m_mockOnCompleteCallback, Call(_, _))
	    .WillOnce(Invoke([](auto const& status, auto const& parameter)
	                     { EXPECT_THAT(status.status, Eq(::middleware::parameter::Status::FAILURE)); }));

	m_spObjUnderTest->Write(TestParameter().data(), m_mockOnCompleteCallback.AsStdFunction());
}

TEST_F(WriteParameterTest, WriteCallbackReceivesParameterWithCorrectIdentityAndUnit)
{
	EXPECT_CALL(*m_spMockMiddlewareClient, Write(_, _))
	    .WillOnce(Invoke(
	        [](auto const& protobuf, auto&& callback)
	        {
		        ::middleware::parameter::Parameter parameter{};
		        parameter.CopyFrom(protobuf);
		        EXPECT_THAT(parameter.info().identity(), Eq(n_SomeParameterName));
		        EXPECT_THAT(parameter.info().unit(), Eq(static_cast<uint32_t>(n_SomeUnit)));
		        parameter.mutable_data()->set_quality(::cat::middleware::parameter::QUALITY_GOOD);

		        ::middleware::Reply reply{};
		        reply.status.status_code  = ::middleware::StatusCode::STATUS_CODE_SUCCESS;
		        reply.serialized_protobuf = ::middleware::common::SerializeToVector(parameter.data());
		        callback(reply);
	        }
	    ));

	EXPECT_CALL(m_mockOnCompleteCallback, Call(_, _))
	    .WillOnce(Invoke(
	        [](auto const& status, auto const& parameter)
	        {
		        EXPECT_THAT(parameter.info().identity(), Eq(n_SomeParameterName));
		        EXPECT_THAT(parameter.info().unit(), Eq(static_cast<uint32_t>(n_SomeUnit)));
	        }
	    ));
	m_spObjUnderTest->Write(TestParameter().data(), m_mockOnCompleteCallback.AsStdFunction());
}

TEST_F(WriteParameterTest, WriteDataIsCopiedToClientWriteCall)
{
	::middleware::parameter::Parameter capturedParameter{};
	EXPECT_CALL(*m_spMockMiddlewareClient, Write(_, _))
	    .WillOnce(Invoke(
	        [&capturedParameter](auto const& protobuf, auto&&)
	        {
		        ::middleware::parameter::Parameter parameter{};
		        parameter.CopyFrom(protobuf);
		        EXPECT_THAT(parameter.data().quality(), Eq(::cat::middleware::parameter::QUALITY_GOOD));
		        EXPECT_THAT(parameter.data().value().string_value(), Eq("Test"));
	        }
	    ));

	::middleware::parameter::Data data{};
	data.mutable_value()->set_string_value("Test");
	data.set_quality(::cat::middleware::parameter::QUALITY_GOOD);

	m_spObjUnderTest->Write(data, [](auto const&, auto const&) {});
}
