// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ReadParameterTest.cpp
// Description: Unit Tests for ReadParameter class

#include "ReadParameter.cpp"
#include "mockMiddlewareClient.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;

class ReadParameterTest : public ::testing::Test
{
protected:
	ReadParameterTest()
	    : m_spMockMiddlewareClient{std::make_shared<NiceMock<mock::MiddlewareClient>>()},
	      m_spObjUnderTest{
	          std::make_unique<::middleware::parameter::ReadParameter>(m_parameterConfig, m_spMockMiddlewareClient)
	      }
	{
	}
	virtual ~ReadParameterTest() = default;

	std::string m_testIdentity{"test.parameter"};
	::middleware::parameter::Unit m_testUnit{::middleware::parameter::Unit::count};

	::middleware::parameter::client::ReadParameter m_parameterConfig{m_testIdentity, m_testUnit};
	std::shared_ptr<NiceMock<mock::MiddlewareClient>> m_spMockMiddlewareClient;
	std::unique_ptr<::middleware::parameter::ReadParameter> m_spObjUnderTest;
};

TEST_F(ReadParameterTest, DeathTestConstructorPassedNullptrMiddlewareClient)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(
	    std::make_unique<::middleware::parameter::ReadParameter>(m_parameterConfig, nullptr), "Assertion .* failed"
	);
}
TEST_F(ReadParameterTest, DeathTestReadPassedNullCallback)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(m_spObjUnderTest->Read(nullptr), "Assertion .* failed");
}
TEST_F(ReadParameterTest, ConstructorSuccessWithValidMiddlewareClient)
{
	EXPECT_NO_THROW(
	    std::make_unique<::middleware::parameter::ReadParameter>(m_parameterConfig, m_spMockMiddlewareClient)
	);
}

TEST_F(ReadParameterTest, ReadCallsMiddlewareClientRead)
{
	EXPECT_CALL(*m_spMockMiddlewareClient, Read(_, _));
	m_spObjUnderTest->Read([](auto const&, auto const&) {});
}

TEST_F(ReadParameterTest, ReadCallbackInvokedWithSuccessStatus)
{
	EXPECT_CALL(*m_spMockMiddlewareClient, Read(_, _))
	    .WillOnce(Invoke(
	        [this](auto const& protobuf, auto&& callback)
	        {
		        ::middleware::parameter::Parameter parameter{};
		        parameter.CopyFrom(protobuf);
		        EXPECT_THAT(parameter.info().identity(), Eq(m_testIdentity));
		        EXPECT_THAT(parameter.info().unit(), Eq(static_cast<uint32_t>(m_testUnit)));
		        parameter.mutable_data()->set_quality(::cat::middleware::parameter::QUALITY_GOOD);

		        ::middleware::Reply reply{};
		        reply.status.status_code  = ::middleware::StatusCode::STATUS_CODE_SUCCESS;
		        reply.serialized_protobuf = ::middleware::common::SerializeToVector(parameter.data());
		        callback(reply);
	        }
	    ));

	m_spObjUnderTest->Read([](auto const& status, auto const&)
	                       { EXPECT_EQ(status.status, ::middleware::parameter::Status::SUCCESS); });
}

TEST_F(ReadParameterTest, ReadCallbackInvokedWithFailureWhenQualityNotGood)
{
	EXPECT_CALL(*m_spMockMiddlewareClient, Read(_, _))
	    .WillOnce(Invoke(
	        [this](auto const& protobuf, auto&& callback)
	        {
		        ::middleware::parameter::Parameter parameter{};
		        parameter.CopyFrom(protobuf);
		        EXPECT_THAT(parameter.info().identity(), Eq(m_testIdentity));
		        EXPECT_THAT(parameter.info().unit(), Eq(static_cast<uint32_t>(m_testUnit)));
		        parameter.mutable_data()->set_quality(::cat::middleware::parameter::QUALITY_BAD);

		        ::middleware::Reply reply{};
		        reply.status.status_code  = ::middleware::StatusCode::STATUS_CODE_SUCCESS;
		        reply.serialized_protobuf = ::middleware::common::SerializeToVector(parameter.data());
		        callback(reply);
	        }
	    ));

	m_spObjUnderTest->Read([](auto const& status, auto const&)
	                       { EXPECT_EQ(status.status, ::middleware::parameter::Status::FAILURE); });
}

TEST_F(ReadParameterTest, ReadCallbackInvokedWithFailureWhenMiddlewareCallFails)
{
	EXPECT_CALL(*m_spMockMiddlewareClient, Read(_, _))
	    .WillOnce(Invoke(
	        [](auto const&, auto&& callback)
	        {
		        ::middleware::Reply reply{};
		        reply.status.status_code    = ::middleware::StatusCode::STATUS_CODE_CLIENT_TIMEOUT_ERROR;
		        reply.status.status_message = "Test failure";
		        callback(reply);
	        }
	    ));

	m_spObjUnderTest->Read([](auto const& status, auto const&)
	                       { EXPECT_EQ(status.status, ::middleware::parameter::Status::FAILURE); });
}
