// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ConfigBuilderTest.cpp
// Description: Unit Tests for ConfigBuilder class

#include "ConfigBuilder.cpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;
namespace
{
static constexpr char n_TestInstanceName[]  = "SomeIdentity";
static constexpr char n_TestParameterName[] = "SomeParameter";
using MockOnWriteFunction                   = NiceMock<MockFunction<std::optional<
                      ::middleware::parameter::
                          Data>(uint_least16_t, ::middleware::parameter::Identity const&, ::middleware::parameter::Data const&)>>;
using MockOnReadFunction                    = NiceMock<MockFunction<
                       std::optional<::middleware::parameter::Data>(uint_least16_t, ::middleware::parameter::Identity const&)>>;
using MockSubscriptionFunction =
    NiceMock<MockFunction<void(::middleware::parameter::Identity const&, ::middleware::parameter::Data const&)>>;
using MockSubscriptionWithMetaFunction = NiceMock<MockFunction<void(
    ::middleware::parameter::Identity const&,
    ::middleware::parameter::Data const&,
    ::middleware::parameter::Metadata const&
)>>;
using MockDisconnectFunction           = NiceMock<MockFunction<void()>>;

}  // namespace

class ConfigBuilderTest : public ::testing::Test
{
protected:
	ConfigBuilderTest()
	    : m_unit{::middleware::parameter::Unit::count},
	      m_spMockOnWriteCallback{std::make_unique<MockOnWriteFunction>()},
	      m_spMockOnReadCallback{std::make_unique<MockOnReadFunction>()},
	      m_spMockSubscriptionCallback{std::make_unique<MockSubscriptionFunction>()},
	      m_spMockSubscriptionWithMetaCallback{std::make_unique<MockSubscriptionWithMetaFunction>()},
	      m_spMockDisconnectCallback{std::make_unique<MockDisconnectFunction>()},
	      m_spObjUnderTest{std::make_unique<::middleware::parameter::ConfigBuilder>(n_TestInstanceName)}
	{
	}
	virtual ~ConfigBuilderTest() = default;

	::middleware::parameter::Unit m_unit;
	std::unique_ptr<MockOnWriteFunction> m_spMockOnWriteCallback;
	std::unique_ptr<MockOnReadFunction> m_spMockOnReadCallback;
	std::unique_ptr<MockSubscriptionFunction> m_spMockSubscriptionCallback;
	std::unique_ptr<MockSubscriptionWithMetaFunction> m_spMockSubscriptionWithMetaCallback;
	std::unique_ptr<MockDisconnectFunction> m_spMockDisconnectCallback;
	std::unique_ptr<::middleware::parameter::ConfigBuilder> m_spObjUnderTest;
};

TEST_F(ConfigBuilderTest, DeathTestConstructorPassedEmptyIdentityString)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(std::make_unique<::middleware::parameter::ConfigBuilder>(""), "Assertion .* failed");
}
TEST_F(ConfigBuilderTest, DeathTestTakeCalledMoreThanOnce)
{
	m_spObjUnderTest->Take();

	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(m_spObjUnderTest->Take(), "Assertion .* failed");
}
TEST_F(ConfigBuilderTest, DeathTestAddPublishCalledAfterTake)
{
	::middleware::parameter::Data data{};
	m_spObjUnderTest->Take();

	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(
	    m_spObjUnderTest->AddPublish(std::string{n_TestParameterName}, m_unit, std::move(data)), "Assertion .* failed"
	);
}
TEST_F(ConfigBuilderTest, DeathTestAddSubscribeWithoutMetaDataCalledAfterTake)
{
	m_spObjUnderTest->Take();

	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(
	    m_spObjUnderTest->AddSubscribe(
	        std::string{n_TestParameterName},
	        m_unit,
	        m_spMockSubscriptionCallback->AsStdFunction(),
	        m_spMockDisconnectCallback->AsStdFunction()
	    ),
	    "Assertion .* failed"
	);
}
TEST_F(ConfigBuilderTest, DeathTestAddSubscribeWithMetaDataCalledAfterTake)
{
	m_spObjUnderTest->Take();

	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(
	    m_spObjUnderTest->AddSubscribe(
	        std::string{n_TestParameterName},
	        m_spMockSubscriptionWithMetaCallback->AsStdFunction(),
	        m_spMockDisconnectCallback->AsStdFunction()
	    ),
	    "Assertion .* failed"
	);
}
TEST_F(ConfigBuilderTest, DeathTestAddClientReadCalledAfterTake)
{
	m_spObjUnderTest->Take();

	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(m_spObjUnderTest->AddClientRead(std::string{n_TestParameterName}, m_unit), "Assertion .* failed");
}
TEST_F(ConfigBuilderTest, DeathTestAddClientWriteCalledAfterTake)
{
	m_spObjUnderTest->Take();

	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(m_spObjUnderTest->AddClientWrite(std::string{n_TestParameterName}, m_unit), "Assertion .* failed");
}
TEST_F(ConfigBuilderTest, DeathTestAddAddServerWriteCalledAfterTake)
{
	m_spObjUnderTest->Take();

	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(
	    m_spObjUnderTest->AddServerWrite(std::string{n_TestParameterName}, m_spMockOnWriteCallback->AsStdFunction()),
	    "Assertion .* failed"
	);
}
TEST_F(ConfigBuilderTest, DeathTestAddAddServerReadCalledAfterTake)
{
	m_spObjUnderTest->Take();

	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(
	    m_spObjUnderTest->AddServerRead(std::string{n_TestParameterName}, m_spMockOnReadCallback->AsStdFunction()),
	    "Assertion .* failed"
	);
}
TEST_F(ConfigBuilderTest, ClassConstructorTakeReturnsThePopulatedConfiguration)
{
	// Callback expectations
	EXPECT_CALL(*m_spMockSubscriptionCallback, Call(_, _));
	EXPECT_CALL(*m_spMockSubscriptionWithMetaCallback, Call(_, _, _));
	EXPECT_CALL(*m_spMockOnWriteCallback, Call(_, _, _));
	EXPECT_CALL(*m_spMockOnReadCallback, Call(_, _));
	EXPECT_CALL(*m_spMockDisconnectCallback, Call()).Times(2);

	::middleware::parameter::Data data{};
	data.mutable_value()->set_string_value("Test");
	data.set_quality(::cat::middleware::parameter::QUALITY_GOOD);
	m_spObjUnderTest->AddPublish("AddPublish", m_unit, std::move(data));
	m_spObjUnderTest->AddSubscribe(
	    "AddSubscribe",
	    m_unit,
	    m_spMockSubscriptionCallback->AsStdFunction(),
	    m_spMockDisconnectCallback->AsStdFunction()
	);
	m_spObjUnderTest->AddSubscribe(
	    "AddSubscribeMeta",
	    m_spMockSubscriptionWithMetaCallback->AsStdFunction(),
	    m_spMockDisconnectCallback->AsStdFunction()
	);
	m_spObjUnderTest->AddClientRead("AddClientRead", m_unit);
	m_spObjUnderTest->AddClientWrite("AddClientWrite", m_unit);
	m_spObjUnderTest->AddServerWrite("AddServerWrite", m_spMockOnWriteCallback->AsStdFunction());
	m_spObjUnderTest->AddServerRead("AddServerRead", m_spMockOnReadCallback->AsStdFunction());
	auto configuration = m_spObjUnderTest->Take();

	// Instance Identity
	EXPECT_THAT(configuration.server_identity, Eq(n_TestInstanceName));

	// Publishers
	ASSERT_THAT(configuration.published_parameters, SizeIs(1));
	EXPECT_THAT(configuration.published_parameters.at(0).parameter_id, Eq("AddPublish"));
	EXPECT_THAT(configuration.published_parameters.at(0).unit, Eq(m_unit));
	EXPECT_THAT(configuration.published_parameters.at(0).initial_value.value().string_value(), Eq("Test"));

	// On Writes
	ASSERT_THAT(configuration.server_write_parameters, SizeIs(1));
	EXPECT_THAT(configuration.server_write_parameters.at(0).parameter_id, Eq("AddServerWrite"));
	configuration.server_write_parameters.at(0).callback(0, "", data);

	// On Reads
	ASSERT_THAT(configuration.server_read_parameters, SizeIs(1));
	EXPECT_THAT(configuration.server_read_parameters.at(0).parameter_id, Eq("AddServerRead"));
	configuration.server_read_parameters.at(0).callback(0, "");

	// Subscribers
	ASSERT_THAT(configuration.subscribed_parameters, SizeIs(2));
	EXPECT_THAT(configuration.subscribed_parameters.at(0).parameter_id, Eq("AddSubscribe"));
	EXPECT_THAT(configuration.subscribed_parameters.at(0).unit, Eq(m_unit));
	EXPECT_FALSE(configuration.subscribed_parameters.at(0).callbackWithMetadata);
	configuration.subscribed_parameters.at(0).callback("", data);
	configuration.subscribed_parameters.at(0).disconnect();

	EXPECT_THAT(configuration.subscribed_parameters.at(1).parameter_id, Eq("AddSubscribeMeta"));
	EXPECT_FALSE(configuration.subscribed_parameters.at(1).callback);
	configuration.subscribed_parameters.at(1).callbackWithMetadata("", data, {});
	configuration.subscribed_parameters.at(1).disconnect();

	// Client Write
	ASSERT_THAT(configuration.client_write_parameters, SizeIs(1));
	EXPECT_THAT(configuration.client_write_parameters.at(0).parameter_id, Eq("AddClientWrite"));
	EXPECT_THAT(configuration.client_write_parameters.at(0).unit, Eq(m_unit));

	// Client Read
	ASSERT_THAT(configuration.client_read_parameters, SizeIs(1));
	EXPECT_THAT(configuration.client_read_parameters.at(0).parameter_id, Eq("AddClientRead"));
	EXPECT_THAT(configuration.client_read_parameters.at(0).unit, Eq(m_unit));
}
