// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: PublishParameterTest.cpp
// Description: Unit Tests for PublishParameter class

#include "PublishParameter.cpp"
#include "mockMiddlewareParticipant.h"
#include "mockMiddlewareWriter.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;
namespace
{
static constexpr char n_SomeParameterName[]    = "Product ID";
static constexpr char n_ParameterChannelName[] = "ParameterService::ProductID";
static constexpr char n_SomeValue[]            = "CAT12345";
static constexpr auto n_SomeUnit               = ::middleware::parameter::Unit::degree;
static ::middleware::parameter::Data SomeData()
{
	::middleware::parameter::Data data{};
	data.mutable_value()->set_string_value(n_SomeValue);
	data.set_quality(::cat::middleware::parameter::QUALITY_GOOD);
	return data;
}
}  // namespace

class PublishParameterTest : public ::testing::Test
{
protected:
	PublishParameterTest()
	    : m_parameterConfig{n_SomeParameterName, n_SomeUnit, SomeData()},
	      m_spMockMiddlewareParticipant{std::make_shared<NiceMock<mock::MiddlewareParticipant>>()},
	      m_pMockMiddlewareWriter{nullptr}
	{
		EXPECT_CALL(*m_spMockMiddlewareParticipant, CreateWriter(_))
		    .WillOnce(Invoke(
		        [this](auto const& id) -> std::unique_ptr<NiceMock<mock::MiddlewareWriter>>
		        {
			        auto writer             = std::make_unique<NiceMock<mock::MiddlewareWriter>>();
			        m_pMockMiddlewareWriter = writer.get();
			        return writer;
		        }
		    ));
		m_spObjUnderTest = std::make_unique<::middleware::parameter::PublishParameter>(
		    m_parameterConfig, m_spMockMiddlewareParticipant
		);
	}
	virtual ~PublishParameterTest() = default;

	::middleware::parameter::server::PublishedParameter m_parameterConfig;
	std::shared_ptr<NiceMock<mock::MiddlewareParticipant>> m_spMockMiddlewareParticipant;
	NiceMock<mock::MiddlewareWriter>* m_pMockMiddlewareWriter;

	std::unique_ptr<::middleware::parameter::PublishParameter> m_spObjUnderTest;
};
TEST_F(PublishParameterTest, DeathTestConstructorPassedNullptrMiddlewareParticipant)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(
	    std::make_unique<::middleware::parameter::PublishParameter>(m_parameterConfig, nullptr), "Assertion .* failed"
	);
}
TEST_F(PublishParameterTest, DeathTestConstructorParticipantReturnsWriterAsNullptr)
{
	auto spMockMiddlewareParticipant        = std::make_shared<NiceMock<mock::MiddlewareParticipant>>();
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(
	    std::make_unique<::middleware::parameter::PublishParameter>(m_parameterConfig, spMockMiddlewareParticipant),
	    "Assertion .* failed"
	);
}
TEST_F(PublishParameterTest, PublishMethodCallsWriterPublishWithCorrectData)
{
	::middleware::parameter::Data newData{};
	newData.mutable_value()->set_string_value("NewValue");
	newData.set_quality(::cat::middleware::parameter::QUALITY_GOOD);

	EXPECT_CALL(*m_pMockMiddlewareWriter, Publish(An<::google::protobuf::Message const&>()))
	    .WillOnce(Invoke(
	        [&](::google::protobuf::Message const& message) -> bool
	        {
		        ::middleware::parameter::Parameter parameter{};
		        parameter.CopyFrom(message);
		        EXPECT_THAT(parameter.data().value().string_value(), Eq("NewValue"));
		        EXPECT_THAT(parameter.data().quality(), Eq(newData.quality()));
		        return true;
	        }
	    ));

	m_spObjUnderTest->Publish(newData);
}

TEST_F(PublishParameterTest, PublishMethodUpdatesLastPublishedData)
{
	::middleware::parameter::Data newData{};
	newData.mutable_value()->set_string_value("UpdatedValue");
	newData.set_quality(::cat::middleware::parameter::QUALITY_BAD);

	EXPECT_CALL(*m_pMockMiddlewareWriter, Publish(An<::google::protobuf::Message const&>())).WillOnce(Return(true));
	m_spObjUnderTest->Publish(newData);

	EXPECT_CALL(*m_pMockMiddlewareWriter, Publish(An<::google::protobuf::Message const&>()))
	    .WillOnce(Invoke(
	        [&](::google::protobuf::Message const& message) -> bool
	        {
		        ::middleware::parameter::Parameter parameter{};
		        parameter.CopyFrom(message);
		        EXPECT_THAT(parameter.data().value().string_value(), Eq("UpdatedValue"));
		        EXPECT_THAT(parameter.data().quality(), Eq(::cat::middleware::parameter::QUALITY_BAD));
		        return true;
	        }
	    ));
	m_spObjUnderTest->Publish(newData);
}

TEST_F(PublishParameterTest, PublishMethodRetainsParameterInfoAcrossPublishes)
{
	::middleware::parameter::Data newData{};
	newData.mutable_value()->set_string_value("AnotherValue");
	newData.set_quality(::cat::middleware::parameter::QUALITY_GOOD);

	EXPECT_CALL(*m_pMockMiddlewareWriter, Publish(An<::google::protobuf::Message const&>()))
	    .WillOnce(Invoke(
	        [this](::google::protobuf::Message const& message) -> bool
	        {
		        ::middleware::parameter::Parameter parameter{};
		        parameter.CopyFrom(message);
		        EXPECT_THAT(parameter.info().identity(), Eq(m_parameterConfig.parameter_id));
		        EXPECT_THAT(parameter.info().unit(), Eq(static_cast<uint32_t>(m_parameterConfig.unit)));
		        return true;
	        }
	    ));

	m_spObjUnderTest->Publish(newData);
}

TEST_F(PublishParameterTest, IdentityReturnsCorrectParameterIdentity)
{
	auto identity = m_spObjUnderTest->Identity();
	EXPECT_THAT(identity, Eq(n_SomeParameterName));
}

TEST_F(PublishParameterTest, UnitReturnsCorrectParameterUnit)
{
	auto unit = m_spObjUnderTest->Unit();
	EXPECT_THAT(unit, Eq(n_SomeUnit));
}

TEST_F(PublishParameterTest, PublishHandlesWriterPublishFailure)
{
	::middleware::parameter::Data newData{};
	newData.mutable_value()->set_string_value("FailValue");
	newData.set_quality(::cat::middleware::parameter::QUALITY_GOOD);

	EXPECT_CALL(*m_pMockMiddlewareWriter, Publish(An<::google::protobuf::Message const&>())).WillOnce(Return(false));

	m_spObjUnderTest->Publish(newData);
}
TEST_F(PublishParameterTest, MultiplePublishCallsSucceed)
{
	EXPECT_CALL(*m_pMockMiddlewareWriter, Publish(An<::google::protobuf::Message const&>()))
	    .Times(3)
	    .WillRepeatedly(Return(true));

	::middleware::parameter::Data data1{};
	data1.mutable_value()->set_string_value("Value1");
	data1.set_quality(::cat::middleware::parameter::QUALITY_GOOD);
	m_spObjUnderTest->Publish(data1);

	::middleware::parameter::Data data2{};
	data2.mutable_value()->set_string_value("Value2");
	data2.set_quality(::cat::middleware::parameter::QUALITY_BAD);
	m_spObjUnderTest->Publish(data2);

	::middleware::parameter::Data data3{};
	data3.mutable_value()->set_string_value("Value3");
	data3.set_quality(::cat::middleware::parameter::QUALITY_GOOD);
	m_spObjUnderTest->Publish(data3);
}
TEST_F(PublishParameterTest, OnConstructionTheExpectedConnectionNameWillBePassedToTheMiddlewareParticipant)
{
	auto spMockMiddlewareParticipant = std::make_shared<NiceMock<mock::MiddlewareParticipant>>();
	EXPECT_CALL(*spMockMiddlewareParticipant, CreateWriter(n_ParameterChannelName))
	    .WillOnce(Invoke([](auto const& id) { return std::make_unique<NiceMock<mock::MiddlewareWriter>>(); }));
	auto spObjUnderTest =
	    std::make_unique<::middleware::parameter::PublishParameter>(m_parameterConfig, spMockMiddlewareParticipant);
	EXPECT_TRUE(spObjUnderTest);
}
TEST_F(PublishParameterTest, OnConstructionInitialDataWillBePublished)
{
	auto spMockMiddlewareParticipant = std::make_shared<NiceMock<mock::MiddlewareParticipant>>();
	EXPECT_CALL(*spMockMiddlewareParticipant, CreateWriter(_))
	    .WillOnce(Invoke(
	        [this](auto const& id)
	        {
		        auto spMockWriter = std::make_unique<NiceMock<mock::MiddlewareWriter>>();

		        EXPECT_CALL(*spMockWriter, Publish(An<::google::protobuf::Message const&>()))
		            .WillOnce(Invoke(
		                [this](::google::protobuf::Message const& message) -> bool
		                {
			                ::middleware::parameter::Parameter parameter{};
			                parameter.CopyFrom(message);
			                EXPECT_THAT(parameter.info().identity(), Eq(m_parameterConfig.parameter_id));
			                EXPECT_THAT(parameter.info().unit(), Eq(static_cast<uint32_t>(m_parameterConfig.unit)));
			                EXPECT_THAT(parameter.data().quality(), Eq(m_parameterConfig.initial_value.quality()));
			                EXPECT_THAT(
			                    parameter.data().value().string_value(),
			                    Eq(m_parameterConfig.initial_value.value().string_value())
			                );
			                return true;
		                }
		            ));
		        return spMockWriter;
	        }
	    ));
	auto spObjUnderTest =
	    std::make_unique<::middleware::parameter::PublishParameter>(m_parameterConfig, spMockMiddlewareParticipant);
	EXPECT_TRUE(spObjUnderTest);
}
