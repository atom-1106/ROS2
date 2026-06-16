// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: SubscribeParameterTest.cpp
// Description: Unit Tests for SubscribeParameter class

#include "SubscribeParameter.cpp"
#include "mockMiddlewareParticipant.h"
#include "mockMiddlewareReader.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;
namespace
{
static constexpr char n_SomeParameterName[]    = "Product ID";
static constexpr char n_ParameterChannelName[] = "ParameterService::ProductID";
static constexpr auto n_SomeUnit               = ::middleware::parameter::Unit::degree;
using MockSubscriptionFunction =
    NiceMock<MockFunction<void(::middleware::parameter::Identity const&, ::middleware::parameter::Data const&)>>;
using MockSubscriptionWithMetaFunction = NiceMock<MockFunction<void(
    ::middleware::parameter::Identity const&,
    ::middleware::parameter::Data const&,
    ::middleware::parameter::Metadata const&
)>>;
using MockDisconnectFunction           = NiceMock<MockFunction<void()>>;

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
static std::vector<uint8_t> SerializedTestParameter(
    ::cat::middleware::parameter::Quality quality = ::cat::middleware::parameter::QUALITY_GOOD
)
{
	auto parameter = TestParameter(quality);
	return ::middleware::common::SerializeToVector(parameter);
}
}  // namespace

class SubscribeParameterTest : public ::testing::Test
{
protected:
	SubscribeParameterTest()
	    : m_spMockSubscriptionCallback{std::make_unique<MockSubscriptionFunction>()},
	      m_spMockSubscriptionWithMetaCallback{std::make_unique<MockSubscriptionWithMetaFunction>()},
	      m_spMockDisconnectCallback{std::make_unique<MockDisconnectFunction>()},
	      m_parameterConfig{
	          n_SomeParameterName,
	          n_SomeUnit,
	          m_spMockSubscriptionCallback->AsStdFunction(),
	          m_spMockSubscriptionWithMetaCallback->AsStdFunction(),
	          m_spMockDisconnectCallback->AsStdFunction()
	      },
	      m_pMockMiddlewareReader{nullptr},
	      m_spMockMiddlewareParticipant{std::make_shared<NiceMock<mock::MiddlewareParticipant>>()}
	{
		EXPECT_CALL(*m_spMockMiddlewareParticipant, CreateReader(_, _, _))
		    .WillOnce(Invoke(
		        [this](auto const& id, auto const& subscriptionCallback, auto const& disconnectCallback)
		            -> std::unique_ptr<NiceMock<mock::MiddlewareReader>>
		        {
			        auto reader                    = std::make_unique<NiceMock<mock::MiddlewareReader>>();
			        m_pMockMiddlewareReader        = reader.get();
			        m_capturedSubscriptionCallback = subscriptionCallback;
			        m_capturedDisconnectCallback   = disconnectCallback;
			        return reader;
		        }
		    ));
		m_spObjUnderTest = std::make_unique<::middleware::parameter::SubscribeParameter>(
		    m_parameterConfig, m_spMockMiddlewareParticipant
		);
	}
	virtual ~SubscribeParameterTest() = default;

	std::unique_ptr<MockSubscriptionFunction> m_spMockSubscriptionCallback;
	std::unique_ptr<MockSubscriptionWithMetaFunction> m_spMockSubscriptionWithMetaCallback;
	std::unique_ptr<MockDisconnectFunction> m_spMockDisconnectCallback;
	::middleware::parameter::client::SubscribedParameter m_parameterConfig;
	NiceMock<mock::MiddlewareReader>* m_pMockMiddlewareReader;
	std::shared_ptr<NiceMock<mock::MiddlewareParticipant>> m_spMockMiddlewareParticipant;

	::middleware::IMiddlewareReader::SubscriptionCallback m_capturedSubscriptionCallback;
	::middleware::IMiddlewareReader::OnDisconnectCallback m_capturedDisconnectCallback;
	std::unique_ptr<::middleware::parameter::SubscribeParameter> m_spObjUnderTest;
};

TEST_F(SubscribeParameterTest, DeathTestConstructorPassedNullptrMiddlewareParticipant)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(
	    std::make_unique<::middleware::parameter::SubscribeParameter>(m_parameterConfig, nullptr), "Assertion .* failed"
	);
}
TEST_F(SubscribeParameterTest, DeathTestConstructorParticipantReturnsReaderAsNullptr)
{
	auto spMockMiddlewareParticipant        = std::make_shared<NiceMock<mock::MiddlewareParticipant>>();
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(
	    std::make_unique<::middleware::parameter::SubscribeParameter>(m_parameterConfig, spMockMiddlewareParticipant),
	    "Assertion .* failed"
	);
}
TEST_F(SubscribeParameterTest, IdentityReturnsConfiguredParameterName)
{
	EXPECT_THAT(m_spObjUnderTest->Identity(), Eq(n_SomeParameterName));
}

TEST_F(SubscribeParameterTest, UnitReturnsZeroUnitWhenSubscriptionCallbackHasNeverBeenCalled)
{
	EXPECT_THAT(m_spObjUnderTest->Unit(), Eq(::middleware::parameter::Unit::none));
}

TEST_F(SubscribeParameterTest, UnitReturnsValidUnitOnlyWhenSubscriptionCallbackIsCalledAtLeastOnce)
{
	m_capturedSubscriptionCallback(SerializedTestParameter());
	EXPECT_THAT(m_spObjUnderTest->Unit(), Eq(n_SomeUnit));
}

TEST_F(SubscribeParameterTest, SubscriptionCallbackIsCalledWhenDataReceived)
{
	EXPECT_CALL(*m_spMockSubscriptionCallback, Call(_, _));

	ASSERT_NE(m_capturedSubscriptionCallback, nullptr);
	m_capturedSubscriptionCallback(SerializedTestParameter());
}

TEST_F(SubscribeParameterTest, SubscriptionWithMetaCallbackIsCalledWhenDataReceived)
{
	EXPECT_CALL(*m_spMockSubscriptionWithMetaCallback, Call(_, _, _));

	ASSERT_NE(m_capturedSubscriptionCallback, nullptr);
	m_capturedSubscriptionCallback(SerializedTestParameter());
}

TEST_F(SubscribeParameterTest, BothSubscriptionCallbacksAreCalledWhenDataReceived)
{
	EXPECT_CALL(*m_spMockSubscriptionCallback, Call(_, _));
	EXPECT_CALL(*m_spMockSubscriptionWithMetaCallback, Call(_, _, _));

	ASSERT_NE(m_capturedSubscriptionCallback, nullptr);
	m_capturedSubscriptionCallback(SerializedTestParameter());
}

TEST_F(SubscribeParameterTest, DisconnectCallbackIsCalledOnDisconnect)
{
	EXPECT_CALL(*m_spMockDisconnectCallback, Call());

	ASSERT_NE(m_capturedDisconnectCallback, nullptr);
	m_capturedDisconnectCallback();
}

TEST_F(SubscribeParameterTest, SubscriptionCallbackIsCalledOnDisconnect)
{
	EXPECT_CALL(*m_spMockSubscriptionCallback, Call(_, _));

	ASSERT_NE(m_capturedDisconnectCallback, nullptr);
	m_capturedDisconnectCallback();
}

TEST_F(SubscribeParameterTest, SubscriptionWithMetaCallbackIsCalledOnDisconnect)
{
	EXPECT_CALL(*m_spMockSubscriptionWithMetaCallback, Call(_, _, _));

	ASSERT_NE(m_capturedDisconnectCallback, nullptr);
	m_capturedDisconnectCallback();
}

TEST_F(SubscribeParameterTest, SubscriptionCallbackIdentityMatchesParameterName)
{
	::middleware::parameter::Identity capturedIdentity{};
	EXPECT_CALL(*m_spMockSubscriptionCallback, Call(_, _))
	    .WillOnce(Invoke([&capturedIdentity](auto const& identity, auto const&) { capturedIdentity = identity; }));

	ASSERT_NE(m_capturedSubscriptionCallback, nullptr);
	m_capturedSubscriptionCallback(SerializedTestParameter());

	EXPECT_THAT(capturedIdentity, Eq(n_SomeParameterName));
}

TEST_F(SubscribeParameterTest, SubscriptionWithMetaCallbackUnitMatchesConfiguredUnit)
{
	::middleware::parameter::Metadata capturedMetadata{::middleware::parameter::Unit::none};
	EXPECT_CALL(*m_spMockSubscriptionWithMetaCallback, Call(_, _, _))
	    .WillOnce(Invoke([&capturedMetadata](auto const&, auto const&, auto const& metadata)
	                     { capturedMetadata = metadata; }));

	ASSERT_NE(m_capturedSubscriptionCallback, nullptr);
	m_capturedSubscriptionCallback(SerializedTestParameter());

	EXPECT_THAT(capturedMetadata.unit, Eq(n_SomeUnit));
}

TEST_F(SubscribeParameterTest, SubscriptionCallbackNotCalledBeforeDataReceived)
{
	EXPECT_CALL(*m_spMockSubscriptionCallback, Call(_, _)).Times(0);
	EXPECT_CALL(*m_spMockSubscriptionWithMetaCallback, Call(_, _, _)).Times(0);
}

TEST_F(SubscribeParameterTest, DisconnectCallbackNotCalledBeforeDisconnect)
{
	EXPECT_CALL(*m_spMockDisconnectCallback, Call()).Times(0);
}

TEST_F(SubscribeParameterTest, SubscriptionCallbackCalledMultipleTimesForMultipleMessages)
{
	EXPECT_CALL(*m_spMockSubscriptionCallback, Call(_, _)).Times(3);

	ASSERT_NE(m_capturedSubscriptionCallback, nullptr);
	m_capturedSubscriptionCallback(SerializedTestParameter());
	m_capturedSubscriptionCallback(SerializedTestParameter());
	m_capturedSubscriptionCallback(SerializedTestParameter());
}

TEST_F(SubscribeParameterTest, ConstructorWithOnlySubscriptionCallbackNoWithMetaCallback)
{
	auto spMockMiddlewareParticipant = std::make_shared<NiceMock<mock::MiddlewareParticipant>>();
	NiceMock<mock::MiddlewareReader>* pReader{nullptr};
	::middleware::IMiddlewareReader::SubscriptionCallback capturedCallback{};

	EXPECT_CALL(*spMockMiddlewareParticipant, CreateReader(_, _, _))
	    .WillOnce(Invoke(
	        [&pReader, &capturedCallback](auto const&, auto const& cb, auto const&)
	            -> std::unique_ptr<NiceMock<mock::MiddlewareReader>>
	        {
		        auto reader      = std::make_unique<NiceMock<mock::MiddlewareReader>>();
		        pReader          = reader.get();
		        capturedCallback = cb;
		        return reader;
	        }
	    ));

	MockSubscriptionFunction mockCallback{};
	::middleware::parameter::client::SubscribedParameter config{
	    n_SomeParameterName,
	    n_SomeUnit,
	    mockCallback.AsStdFunction(),
	    nullptr,
	    m_spMockDisconnectCallback->AsStdFunction()
	};

	auto objUnderTest =
	    std::make_unique<::middleware::parameter::SubscribeParameter>(config, spMockMiddlewareParticipant);

	EXPECT_CALL(mockCallback, Call(_, _));
	capturedCallback(SerializedTestParameter());
}

TEST_F(SubscribeParameterTest, ConstructorWithOnlyWithMetaCallbackNoSubscriptionCallback)
{
	auto spMockMiddlewareParticipant = std::make_shared<NiceMock<mock::MiddlewareParticipant>>();
	NiceMock<mock::MiddlewareReader>* pReader{nullptr};
	::middleware::IMiddlewareReader::SubscriptionCallback capturedCallback{};

	EXPECT_CALL(*spMockMiddlewareParticipant, CreateReader(_, _, _))
	    .WillOnce(Invoke(
	        [&pReader, &capturedCallback](auto const&, auto const& cb, auto const&)
	            -> std::unique_ptr<NiceMock<mock::MiddlewareReader>>
	        {
		        auto reader      = std::make_unique<NiceMock<mock::MiddlewareReader>>();
		        pReader          = reader.get();
		        capturedCallback = cb;
		        return reader;
	        }
	    ));

	MockSubscriptionWithMetaFunction mockCallback{};
	::middleware::parameter::client::SubscribedParameter config{
	    n_SomeParameterName,
	    n_SomeUnit,
	    nullptr,
	    mockCallback.AsStdFunction(),
	    m_spMockDisconnectCallback->AsStdFunction()
	};

	auto objUnderTest =
	    std::make_unique<::middleware::parameter::SubscribeParameter>(config, spMockMiddlewareParticipant);

	EXPECT_CALL(mockCallback, Call(_, _, _));
	capturedCallback(SerializedTestParameter());
}
TEST_F(SubscribeParameterTest, OnConstructionTheExpectedConnectionNameWillBePassedToTheMiddlewareParticipant)
{
	auto spMockMiddlewareParticipant = std::make_shared<NiceMock<mock::MiddlewareParticipant>>();
	EXPECT_CALL(*spMockMiddlewareParticipant, CreateReader(n_ParameterChannelName, _, _))
	    .WillOnce(Invoke([](auto const& id, auto const& cb, auto const&)
	                     { return std::make_unique<NiceMock<mock::MiddlewareReader>>(); }));
	auto spObjUnderTest =
	    std::make_unique<::middleware::parameter::SubscribeParameter>(m_parameterConfig, spMockMiddlewareParticipant);
	EXPECT_TRUE(spObjUnderTest);
}
