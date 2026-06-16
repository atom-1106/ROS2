// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ParameterServiceTest.cpp
// Description: Unit Tests for ParameterService class

#include "MiddlewareFrameworkContext.h"
#include "ParameterService.cpp"
#include "mockMiddlewareReader.h"
#include "mockMiddlewareWriter.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>

using namespace ::testing;

// Known identities and parameter names from the default source table loaded by Domain::Load().
// server_identity="DataLinkEngine" owns these parameters (server / publisher tests).
// server_identity="CoreTelematics" treats them as remote client parameters from DataLinkEngine.
namespace
{
static constexpr char n_ServerIdentityDataLinkEngine[]  = "DataLinkEngine";
static constexpr char n_ServerIdentityCoreTelematrics[] = "CoreTelematics";
static constexpr char n_ParameterEngineSpeed[]          = "EngineSpeed";
static constexpr char n_ParameterBucketHeight[]         = "BucketHeight";
static constexpr char n_ParameterUnknown[]              = "NotAConfiguredParameter";
}  // namespace

class ParameterServiceTest : public ::testing::Test
{
protected:
	ParameterServiceTest()                   = default;
	virtual ~ParameterServiceTest() override = default;

	void TearDown() override
	{
		// Destroy the service before clearing the context so mock expectations are verified while mocks are alive.
		m_spObjUnderTest.reset();
		::test::MiddlewareFrameworkContext::Get().Clear();
	}

	// Every construction needs at minimum one broadcaster participant and one client participant.
	// The default fallback in Domain::GetClientConnections always creates a DataLinkEngine client even when no
	// client parameters are configured, so we must provide at least one client in the context.
	void AddDefaultParticipantAndClient()
	{
		::test::MiddlewareFrameworkContext::Get().AddParticipant(
		    std::make_unique<NiceMock<mock::MiddlewareParticipant>>()
		);
		::test::MiddlewareFrameworkContext::Get().AddClient(std::make_unique<NiceMock<mock::MiddlewareClient>>());
	}

	// Creates a NiceMock writer with a standing Publish expectation. Used wherever a participant
	// needs to satisfy PublishParameter construction (which calls Publish once with the initial value).
	static std::unique_ptr<NiceMock<mock::MiddlewareWriter>> MakeWriterMock()
	{
		auto writer = std::make_unique<NiceMock<mock::MiddlewareWriter>>();
		EXPECT_CALL(*writer, Publish(A<::google::protobuf::Message const&>())).WillRepeatedly(Return(true));
		return writer;
	}

	// Adds a NiceMock participant (with a single CreateWriter → MakeWriterMock) and a NiceMock client
	// to the context. Covers the common setup needed by every RetrievePublisherInstance test.
	void AddParticipantWithWriterAndClient()
	{
		auto mockParticipant = std::make_unique<NiceMock<mock::MiddlewareParticipant>>();
		EXPECT_CALL(*mockParticipant, CreateWriter(_)).WillOnce(Return(ByMove(MakeWriterMock())));
		::test::MiddlewareFrameworkContext::Get().AddParticipant(std::move(mockParticipant));
		::test::MiddlewareFrameworkContext::Get().AddClient(std::make_unique<NiceMock<mock::MiddlewareClient>>());
	}

	// Config helpers — set server identity and push a single well-known parameter entry.
	void ConfigurePublisherService()
	{
		m_parameterConfig.server_identity = n_ServerIdentityDataLinkEngine;
		m_parameterConfig.published_parameters.push_back(
		    {n_ParameterEngineSpeed, ::middleware::parameter::Unit::none, {}}
		);
	}
	void ConfigureClientReadService()
	{
		m_parameterConfig.server_identity = n_ServerIdentityCoreTelematrics;
		m_parameterConfig.client_read_parameters.push_back(
		    {n_ParameterEngineSpeed, ::middleware::parameter::Unit::none}
		);
	}
	void ConfigureClientWriteService()
	{
		m_parameterConfig.server_identity = n_ServerIdentityCoreTelematrics;
		m_parameterConfig.client_write_parameters.push_back(
		    {n_ParameterEngineSpeed, ::middleware::parameter::Unit::none}
		);
	}

	::middleware::parameter::ParameterService m_parameterConfig{};
	std::unique_ptr<::middleware::parameter::Service> m_spObjUnderTest;
};

// ---------------------------------------------------------------------------
// Construction: null participant (death test)
// ---------------------------------------------------------------------------

TEST_F(ParameterServiceTest, DeathTestConstructorFrameworkFactoryReturnsNullptr)
{
	::test::MiddlewareFrameworkContext::Get().AddParticipant(nullptr);
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(std::make_unique<::middleware::parameter::Service>(m_parameterConfig), "Assertion .* failed");
}

// ---------------------------------------------------------------------------
// Construction: minimal (no server / publisher / subscriber / client params)
// ---------------------------------------------------------------------------

TEST_F(ParameterServiceTest, ConstructorWithMinimalConfigurationSucceeds)
{
	AddDefaultParticipantAndClient();

	m_parameterConfig.server_identity = n_ServerIdentityDataLinkEngine;

	ASSERT_NO_THROW(m_spObjUnderTest = std::make_unique<::middleware::parameter::Service>(m_parameterConfig));
	EXPECT_NE(m_spObjUnderTest, nullptr);
}

// ---------------------------------------------------------------------------
// Construction: server creation
// ---------------------------------------------------------------------------

TEST_F(ParameterServiceTest, ConstructorSkipsServerCreationWhenNoServerParametersConfigured)
{
	// Pre-populate the server queue; after construction it must remain unconsumed.
	::test::MiddlewareFrameworkContext::Get().AddServer(std::make_unique<NiceMock<mock::MiddlewareServer>>());

	AddDefaultParticipantAndClient();
	m_parameterConfig.server_identity = n_ServerIdentityDataLinkEngine;

	m_spObjUnderTest = std::make_unique<::middleware::parameter::Service>(m_parameterConfig);

	// The server mock was never taken from the context, proving CreateServerParticipant was not called.
	EXPECT_NE(::test::MiddlewareFrameworkContext::Get().TakeServer(), nullptr);
}

TEST_F(ParameterServiceTest, ConstructorCreatesAndRunsServerWhenServerReadParametersConfigured)
{
	auto mockServer = std::make_unique<NiceMock<mock::MiddlewareServer>>();
	auto* serverPtr = mockServer.get();
	EXPECT_CALL(*serverPtr, Run()).Times(1);
	::test::MiddlewareFrameworkContext::Get().AddServer(std::move(mockServer));

	AddDefaultParticipantAndClient();

	m_parameterConfig.server_identity = n_ServerIdentityDataLinkEngine;
	m_parameterConfig.server_read_parameters.push_back(
	    {n_ParameterEngineSpeed,
	     [](auto, auto) -> std::optional<::middleware::parameter::Data>
	     {
		     return {};
	     }}
	);

	ASSERT_NO_THROW(m_spObjUnderTest = std::make_unique<::middleware::parameter::Service>(m_parameterConfig));
}

TEST_F(ParameterServiceTest, ConstructorCreatesAndRunsServerWhenServerWriteParametersConfigured)
{
	auto mockServer = std::make_unique<NiceMock<mock::MiddlewareServer>>();
	auto* serverPtr = mockServer.get();
	EXPECT_CALL(*serverPtr, Run()).Times(1);
	::test::MiddlewareFrameworkContext::Get().AddServer(std::move(mockServer));

	AddDefaultParticipantAndClient();

	m_parameterConfig.server_identity = n_ServerIdentityDataLinkEngine;
	m_parameterConfig.server_write_parameters.push_back(
	    {n_ParameterEngineSpeed,
	     [](auto, auto, auto) -> std::optional<::middleware::parameter::Data>
	     {
		     return {};
	     }}
	);

	ASSERT_NO_THROW(m_spObjUnderTest = std::make_unique<::middleware::parameter::Service>(m_parameterConfig));
}

// ---------------------------------------------------------------------------
// Construction: publishers
// ---------------------------------------------------------------------------

TEST_F(ParameterServiceTest, ConstructorCreatesParticipantWriterForEachPublishedParameter)
{
	auto mockParticipant = std::make_unique<NiceMock<mock::MiddlewareParticipant>>();
	auto* participantPtr = mockParticipant.get();

	// The participant is expected to create exactly one writer for EngineSpeed.
	EXPECT_CALL(*participantPtr, CreateWriter(HasSubstr(n_ParameterEngineSpeed)))
	    .Times(1)
	    .WillOnce(Return(ByMove(MakeWriterMock())));

	::test::MiddlewareFrameworkContext::Get().AddParticipant(std::move(mockParticipant));
	::test::MiddlewareFrameworkContext::Get().AddClient(std::make_unique<NiceMock<mock::MiddlewareClient>>());

	ConfigurePublisherService();
	ASSERT_NO_THROW(m_spObjUnderTest = std::make_unique<::middleware::parameter::Service>(m_parameterConfig));
}

TEST_F(ParameterServiceTest, ConstructorCreatesParticipantWriterForEachOfMultiplePublishedParameters)
{
	auto mockParticipant = std::make_unique<NiceMock<mock::MiddlewareParticipant>>();
	auto* participantPtr = mockParticipant.get();

	// Two separate writers are created, one per published parameter.
	EXPECT_CALL(*participantPtr, CreateWriter(_))
	    .WillOnce(Return(ByMove(MakeWriterMock())))
	    .WillOnce(Return(ByMove(MakeWriterMock())));

	::test::MiddlewareFrameworkContext::Get().AddParticipant(std::move(mockParticipant));
	::test::MiddlewareFrameworkContext::Get().AddClient(std::make_unique<NiceMock<mock::MiddlewareClient>>());

	m_parameterConfig.server_identity = n_ServerIdentityDataLinkEngine;
	m_parameterConfig.published_parameters.push_back({n_ParameterEngineSpeed, ::middleware::parameter::Unit::none, {}});
	m_parameterConfig.published_parameters.push_back(
	    {n_ParameterBucketHeight, ::middleware::parameter::Unit::none, {}}
	);

	ASSERT_NO_THROW(m_spObjUnderTest = std::make_unique<::middleware::parameter::Service>(m_parameterConfig));
}

// ---------------------------------------------------------------------------
// Construction: subscribers
// ---------------------------------------------------------------------------

TEST_F(ParameterServiceTest, ConstructorCreatesParticipantReaderForEachSubscribedParameter)
{
	auto mockParticipant = std::make_unique<NiceMock<mock::MiddlewareParticipant>>();
	auto* participantPtr = mockParticipant.get();

	// The participant is expected to create exactly one reader for EngineSpeed.
	EXPECT_CALL(*participantPtr, CreateReader(HasSubstr(n_ParameterEngineSpeed), _, _))
	    .Times(1)
	    .WillOnce(Return(ByMove(std::make_unique<NiceMock<mock::MiddlewareReader>>())));

	::test::MiddlewareFrameworkContext::Get().AddParticipant(std::move(mockParticipant));
	::test::MiddlewareFrameworkContext::Get().AddClient(std::make_unique<NiceMock<mock::MiddlewareClient>>());

	m_parameterConfig.server_identity = n_ServerIdentityDataLinkEngine;
	m_parameterConfig.subscribed_parameters.push_back(
	    {n_ParameterEngineSpeed, ::middleware::parameter::Unit::none, nullptr, nullptr, nullptr}
	);

	ASSERT_NO_THROW(m_spObjUnderTest = std::make_unique<::middleware::parameter::Service>(m_parameterConfig));
}

// ---------------------------------------------------------------------------
// RetrievePublisherInstance
// ---------------------------------------------------------------------------

TEST_F(ParameterServiceTest, RetrievePublisherInstanceReturnsConfiguredPublisher)
{
	AddParticipantWithWriterAndClient();
	ConfigurePublisherService();
	m_spObjUnderTest = std::make_unique<::middleware::parameter::Service>(m_parameterConfig);

	ASSERT_NO_THROW(m_spObjUnderTest->RetrievePublisherInstance(n_ParameterEngineSpeed));
}

TEST_F(ParameterServiceTest, RetrievePublisherInstanceThrowsForUnknownIdentity)
{
	AddParticipantWithWriterAndClient();
	ConfigurePublisherService();
	m_spObjUnderTest = std::make_unique<::middleware::parameter::Service>(m_parameterConfig);

	ASSERT_THROW(m_spObjUnderTest->RetrievePublisherInstance(n_ParameterUnknown), std::runtime_error);
}

// ---------------------------------------------------------------------------
// RetrieveReadInstance
// ---------------------------------------------------------------------------
// Uses server_identity="CoreTelematics" so that EngineSpeed (owned by DataLinkEngine) is discovered
// as a client connection from a remote server, which causes a ReadParameter to be created.

TEST_F(ParameterServiceTest, RetrieveReadInstanceReturnsConfiguredReadClient)
{
	AddDefaultParticipantAndClient();
	ConfigureClientReadService();
	m_spObjUnderTest = std::make_unique<::middleware::parameter::Service>(m_parameterConfig);

	ASSERT_NO_THROW(m_spObjUnderTest->RetrieveReadInstance(n_ParameterEngineSpeed));
}

TEST_F(ParameterServiceTest, RetrieveReadInstanceThrowsForUnknownIdentity)
{
	AddDefaultParticipantAndClient();
	ConfigureClientReadService();
	m_spObjUnderTest = std::make_unique<::middleware::parameter::Service>(m_parameterConfig);

	ASSERT_THROW(m_spObjUnderTest->RetrieveReadInstance(n_ParameterUnknown), std::runtime_error);
}

// ---------------------------------------------------------------------------
// RetrieveWriteInstance
// ---------------------------------------------------------------------------

TEST_F(ParameterServiceTest, RetrieveWriteInstanceReturnsConfiguredWriteClient)
{
	AddDefaultParticipantAndClient();
	ConfigureClientWriteService();
	m_spObjUnderTest = std::make_unique<::middleware::parameter::Service>(m_parameterConfig);

	ASSERT_NO_THROW(m_spObjUnderTest->RetrieveWriteInstance(n_ParameterEngineSpeed));
}

TEST_F(ParameterServiceTest, RetrieveWriteInstanceThrowsForUnknownIdentity)
{
	AddDefaultParticipantAndClient();
	ConfigureClientWriteService();
	m_spObjUnderTest = std::make_unique<::middleware::parameter::Service>(m_parameterConfig);

	ASSERT_THROW(m_spObjUnderTest->RetrieveWriteInstance(n_ParameterUnknown), std::runtime_error);
}

// ---------------------------------------------------------------------------
// Reply
// ---------------------------------------------------------------------------

TEST_F(ParameterServiceTest, ReplyDelegatesToServerParticipantWithCorrectKey)
{
	static constexpr uint_least16_t n_TestKey{42u};

	auto mockServer = std::make_unique<NiceMock<mock::MiddlewareServer>>();
	auto* serverPtr = mockServer.get();
	EXPECT_CALL(*serverPtr, Run()).Times(1);
	EXPECT_CALL(*serverPtr, Reply(n_TestKey, _)).Times(1);
	::test::MiddlewareFrameworkContext::Get().AddServer(std::move(mockServer));

	AddDefaultParticipantAndClient();

	m_parameterConfig.server_identity = n_ServerIdentityDataLinkEngine;
	m_parameterConfig.server_read_parameters.push_back(
	    {n_ParameterEngineSpeed,
	     [](auto, auto) -> std::optional<::middleware::parameter::Data>
	     {
		     return {};
	     }}
	);
	m_spObjUnderTest = std::make_unique<::middleware::parameter::Service>(m_parameterConfig);

	::middleware::parameter::Data data{};
	ASSERT_NO_THROW(m_spObjUnderTest->Reply(n_TestKey, data));
}
