// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: DiagnosticServiceTest.cpp
// Description: Unit Tests for DiagnosticService class

#include "DiagnosticService.cpp"
#include "MiddlewareFrameworkContext.h"
#include "mockMiddlewareClient.h"
#include "mockMiddlewareParticipant.h"
#include "mockMiddlewareReader.h"
#include "mockMiddlewareServer.h"
#include "mockMiddlewareWriter.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

using namespace ::testing;

class DiagnosticServiceTest : public ::testing::Test
{
protected:
	DiagnosticServiceTest()                   = default;
	virtual ~DiagnosticServiceTest() override = default;

	void TearDown() override
	{
		// Destroy the service before clearing the context so mock expectations are verified while mocks are alive.
		m_spObjUnderTest.reset();
		::test::MiddlewareFrameworkContext::Get().Clear();
	}

	// Creates a NiceMock writer with a standing Publish expectation. Used wherever
	// a participant needs to satisfy CodeServer construction (which calls Publish once
	// with the initial codes).
	static std::unique_ptr<NiceMock<mock::MiddlewareWriter>> MakeWriterMock()
	{
		auto writer = std::make_unique<NiceMock<mock::MiddlewareWriter>>();
		EXPECT_CALL(*writer, Publish(A<::google::protobuf::Message const&>())).WillRepeatedly(Return(true));
		return writer;
	}

	// Adds a plain NiceMock participant (no specific call expectations) to the context.
	void AddParticipant()
	{
		::test::MiddlewareFrameworkContext::Get().AddParticipant(
		    std::make_unique<NiceMock<mock::MiddlewareParticipant>>()
		);
	}

	// Adds a NiceMock participant that expects CreateWriter to be called once,
	// returning a valid writer mock. Used for CreateCodeServer paths.
	void AddParticipantWithWriterExpectation()
	{
		auto mockParticipant = std::make_unique<NiceMock<mock::MiddlewareParticipant>>();
		EXPECT_CALL(*mockParticipant, CreateWriter(_)).WillOnce(Return(ByMove(MakeWriterMock())));
		::test::MiddlewareFrameworkContext::Get().AddParticipant(std::move(mockParticipant));
	}

	// Adds a NiceMock participant that expects CreateReader to be called once,
	// returning a valid reader mock. Used for CreateCodeClient paths.
	void AddParticipantWithReaderExpectation()
	{
		auto mockParticipant = std::make_unique<NiceMock<mock::MiddlewareParticipant>>();
		EXPECT_CALL(*mockParticipant, CreateReader(_, _, _))
		    .WillOnce(Return(ByMove(std::make_unique<NiceMock<mock::MiddlewareReader>>())));
		::test::MiddlewareFrameworkContext::Get().AddParticipant(std::move(mockParticipant));
	}

	// Builds a CodeConsumer with valid (no-op) callback and disconnect functions.
	static ::middleware::diagnostics::CodeConsumer MakeValidConsumer()
	{
		return {
                    [](::cat::middleware::diagnostics::Codes const&) {},
                    []() {}};
	}

	// Builds a valid OnRequest callback for CreateCodeServer.
	static ::middleware::diagnostics::ICodeServer::OnRequest MakeValidOnRequest()
	{
		return [](
		           uint_least16_t, ::cat::middleware::diagnostics::AdditionalInformation const&
		       ) -> ::middleware::diagnostics::ICodeServer::OptionalReply
		{
			return {};
		};
	}

	std::unique_ptr<::middleware::diagnostics::Service> m_spObjUnderTest;
};

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST_F(DiagnosticServiceTest, DeathTestConstructorFrameworkFactoryReturnsNullptr)
{
	::test::MiddlewareFrameworkContext::Get().AddParticipant(nullptr);
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	ASSERT_DEATH(std::make_unique<::middleware::diagnostics::Service>(), "Assertion .* failed");
}

TEST_F(DiagnosticServiceTest, ConstructorSucceedsWithValidParticipant)
{
	AddParticipant();
	ASSERT_NO_THROW(m_spObjUnderTest = std::make_unique<::middleware::diagnostics::Service>());
	EXPECT_NE(m_spObjUnderTest, nullptr);
}

// ---------------------------------------------------------------------------
// CreateCodeClient
// ---------------------------------------------------------------------------

TEST_F(DiagnosticServiceTest, DeathTestCreateCodeClientNullCallback)
{
	AddParticipant();
	m_spObjUnderTest = std::make_unique<::middleware::diagnostics::Service>();

	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	::middleware::diagnostics::CodeConsumer consumer{nullptr, []() {}};
	ASSERT_DEATH(m_spObjUnderTest->CreateCodeClient(std::move(consumer)), "Assertion .* failed");
}

TEST_F(DiagnosticServiceTest, DeathTestCreateCodeClientNullDisconnect)
{
	AddParticipant();
	m_spObjUnderTest = std::make_unique<::middleware::diagnostics::Service>();

	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	::middleware::diagnostics::CodeConsumer consumer{[](auto const&) {}, nullptr};
	ASSERT_DEATH(m_spObjUnderTest->CreateCodeClient(std::move(consumer)), "Assertion .* failed");
}

TEST_F(DiagnosticServiceTest, CreateCodeClientSucceeds)
{
	AddParticipantWithReaderExpectation();
	::test::MiddlewareFrameworkContext::Get().AddClient(std::make_unique<NiceMock<mock::MiddlewareClient>>());

	m_spObjUnderTest = std::make_unique<::middleware::diagnostics::Service>();

	std::unique_ptr<::middleware::diagnostics::ICodeClient> spClient;
	ASSERT_NO_THROW(spClient = m_spObjUnderTest->CreateCodeClient(MakeValidConsumer()));
	EXPECT_NE(spClient, nullptr);
}

TEST_F(DiagnosticServiceTest, CreateCodeClientCallsCreateReaderOnMessageParticipant)
{
	auto mockParticipant = std::make_unique<NiceMock<mock::MiddlewareParticipant>>();
	auto* participantPtr = mockParticipant.get();

	EXPECT_CALL(*participantPtr, CreateReader(HasSubstr(::middleware::diagnostics::connection_name), _, _))
	    .Times(1)
	    .WillOnce(Return(ByMove(std::make_unique<NiceMock<mock::MiddlewareReader>>())));

	::test::MiddlewareFrameworkContext::Get().AddParticipant(std::move(mockParticipant));
	::test::MiddlewareFrameworkContext::Get().AddClient(std::make_unique<NiceMock<mock::MiddlewareClient>>());

	m_spObjUnderTest = std::make_unique<::middleware::diagnostics::Service>();
	ASSERT_NO_THROW(m_spObjUnderTest->CreateCodeClient(MakeValidConsumer()));
}

// ---------------------------------------------------------------------------
// CreateCodeServer
// ---------------------------------------------------------------------------

TEST_F(DiagnosticServiceTest, DeathTestCreateCodeServerNullOnRequest)
{
	AddParticipant();
	m_spObjUnderTest = std::make_unique<::middleware::diagnostics::Service>();

	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	::cat::middleware::diagnostics::Codes codes{};
	ASSERT_DEATH(m_spObjUnderTest->CreateCodeServer(codes, nullptr), "Assertion .* failed");
}

TEST_F(DiagnosticServiceTest, CreateCodeServerSucceeds)
{
	AddParticipantWithWriterExpectation();
	auto mockServer = std::make_unique<NiceMock<mock::MiddlewareServer>>();
	::test::MiddlewareFrameworkContext::Get().AddServer(std::move(mockServer));

	m_spObjUnderTest = std::make_unique<::middleware::diagnostics::Service>();

	::cat::middleware::diagnostics::Codes codes{};
	std::unique_ptr<::middleware::diagnostics::ICodeServer> spServer;
	ASSERT_NO_THROW(spServer = m_spObjUnderTest->CreateCodeServer(codes, MakeValidOnRequest()));
	EXPECT_NE(spServer, nullptr);
}

TEST_F(DiagnosticServiceTest, CreateCodeServerCallsRunOnServerParticipant)
{
	AddParticipantWithWriterExpectation();

	auto mockServer = std::make_unique<NiceMock<mock::MiddlewareServer>>();
	auto* serverPtr = mockServer.get();
	EXPECT_CALL(*serverPtr, Run()).Times(1);
	::test::MiddlewareFrameworkContext::Get().AddServer(std::move(mockServer));

	m_spObjUnderTest = std::make_unique<::middleware::diagnostics::Service>();

	::cat::middleware::diagnostics::Codes codes{};
	ASSERT_NO_THROW(m_spObjUnderTest->CreateCodeServer(codes, MakeValidOnRequest()));
}

TEST_F(DiagnosticServiceTest, CreateCodeServerPublishesInitialCodesOnCreation)
{
	auto mockParticipant = std::make_unique<NiceMock<mock::MiddlewareParticipant>>();
	auto mockWriter      = std::make_unique<NiceMock<mock::MiddlewareWriter>>();
	auto* writerPtr      = mockWriter.get();

	EXPECT_CALL(*writerPtr, Publish(A<::google::protobuf::Message const&>())).Times(1).WillOnce(Return(true));

	EXPECT_CALL(*mockParticipant, CreateWriter(_)).WillOnce(Return(ByMove(std::move(mockWriter))));

	::test::MiddlewareFrameworkContext::Get().AddParticipant(std::move(mockParticipant));
	::test::MiddlewareFrameworkContext::Get().AddServer(std::make_unique<NiceMock<mock::MiddlewareServer>>());

	m_spObjUnderTest = std::make_unique<::middleware::diagnostics::Service>();

	::cat::middleware::diagnostics::Codes codes{};
	ASSERT_NO_THROW(m_spObjUnderTest->CreateCodeServer(codes, MakeValidOnRequest()));
}

TEST_F(DiagnosticServiceTest, CreateCodeServerCallsCreateWriterOnMessageParticipant)
{
	auto mockParticipant = std::make_unique<NiceMock<mock::MiddlewareParticipant>>();
	auto* participantPtr = mockParticipant.get();

	EXPECT_CALL(*participantPtr, CreateWriter(HasSubstr(::middleware::diagnostics::connection_name)))
	    .Times(1)
	    .WillOnce(Return(ByMove(MakeWriterMock())));

	::test::MiddlewareFrameworkContext::Get().AddParticipant(std::move(mockParticipant));
	::test::MiddlewareFrameworkContext::Get().AddServer(std::make_unique<NiceMock<mock::MiddlewareServer>>());

	m_spObjUnderTest = std::make_unique<::middleware::diagnostics::Service>();

	::cat::middleware::diagnostics::Codes codes{};
	ASSERT_NO_THROW(m_spObjUnderTest->CreateCodeServer(codes, MakeValidOnRequest()));
}
