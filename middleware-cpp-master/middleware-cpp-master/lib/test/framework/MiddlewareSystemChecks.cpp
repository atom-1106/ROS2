// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareSystemChecks.cpp
// Description: Dummy Test to Check Software setup.

#include "mockBasicParameter.h"
#include "mockConfigBuilder.h"
#include "mockConfigTaker.h"
#include "mockParameterService.h"
#include "mockPublishParameter.h"
#include "mockReadParameter.h"
#include "mockWriteParameter.h"

#include "mockMiddlewareClient.h"
#include "mockMiddlewareParticipant.h"
#include "mockMiddlewareReader.h"
#include "mockMiddlewareServer.h"
#include "mockMiddlewareWriter.h"

#include "MiddlewareFrameworkContext.h"
#include "ParameterServiceContext.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

using namespace ::testing;
namespace
{
}  // namespace

class MiddlewareSystemChecks : public ::testing::Test
{
protected:
	MiddlewareSystemChecks()          = default;
	virtual ~MiddlewareSystemChecks() = default;

	std::unique_ptr<mock::BasicParameter> m_spMockBasicParameter;
	std::unique_ptr<mock::ConfigBuilder> m_spMockConfigBuilder;
	std::unique_ptr<mock::ConfigTaker> m_spMockConfigTaker;
	std::unique_ptr<mock::ParameterService> m_spMockService;
	std::unique_ptr<mock::PublishParameter> m_spMockPublishParameter;
	std::unique_ptr<mock::ReadParameter> m_spMockReadParameter;
	std::unique_ptr<mock::WriteParameter> m_spMockWriteParameter;

	std::unique_ptr<mock::MiddlewareClient> m_spMockMiddlewareClient;
	std::unique_ptr<mock::MiddlewareServer> m_spMockMiddlewareServer;
	std::unique_ptr<mock::MiddlewareParticipant> m_spMockMiddlewareParticipant;
	std::unique_ptr<mock::MiddlewareReader> m_spMockMiddlewareReader;
	std::unique_ptr<mock::MiddlewareWriter> m_spMockMiddlewareWriter;
};

TEST_F(MiddlewareSystemChecks, MocksCanBeCreated)
{
	m_spMockBasicParameter   = std::make_unique<NiceMock<mock::BasicParameter>>();
	m_spMockConfigBuilder    = std::make_unique<NiceMock<mock::ConfigBuilder>>();
	m_spMockConfigTaker      = std::make_unique<NiceMock<mock::ConfigTaker>>();
	m_spMockService          = std::make_unique<NiceMock<mock::ParameterService>>();
	m_spMockPublishParameter = std::make_unique<NiceMock<mock::PublishParameter>>();
	m_spMockReadParameter    = std::make_unique<NiceMock<mock::ReadParameter>>();
	m_spMockWriteParameter   = std::make_unique<NiceMock<mock::WriteParameter>>();

	m_spMockMiddlewareClient      = std::make_unique<NiceMock<mock::MiddlewareClient>>();
	m_spMockMiddlewareServer      = std::make_unique<NiceMock<mock::MiddlewareServer>>();
	m_spMockMiddlewareParticipant = std::make_unique<NiceMock<mock::MiddlewareParticipant>>();
	m_spMockMiddlewareReader      = std::make_unique<NiceMock<mock::MiddlewareReader>>();
	m_spMockMiddlewareWriter      = std::make_unique<NiceMock<mock::MiddlewareWriter>>();

	EXPECT_TRUE(m_spMockBasicParameter);
	EXPECT_TRUE(m_spMockConfigBuilder);
	EXPECT_TRUE(m_spMockConfigTaker);
	EXPECT_TRUE(m_spMockService);
	EXPECT_TRUE(m_spMockPublishParameter);
	EXPECT_TRUE(m_spMockReadParameter);
	EXPECT_TRUE(m_spMockWriteParameter);

	EXPECT_TRUE(m_spMockMiddlewareClient);
	EXPECT_TRUE(m_spMockMiddlewareServer);
	EXPECT_TRUE(m_spMockMiddlewareParticipant);
	EXPECT_TRUE(m_spMockMiddlewareReader);
	EXPECT_TRUE(m_spMockMiddlewareWriter);
}
TEST_F(MiddlewareSystemChecks, FakeFrameworkContextCanBeUsed)
{
	m_spMockMiddlewareClient      = std::make_unique<NiceMock<mock::MiddlewareClient>>();
	m_spMockMiddlewareServer      = std::make_unique<NiceMock<mock::MiddlewareServer>>();
	m_spMockMiddlewareParticipant = std::make_unique<NiceMock<mock::MiddlewareParticipant>>();

	EXPECT_CALL(*m_spMockMiddlewareParticipant, CreateWriter(_))
	    .WillOnce(InvokeWithoutArgs(
	        []() -> std::unique_ptr<::middleware::IMiddlewareWriter>
	        {
		        std::vector<uint8_t> bytes{0x01, 0x02};
		        auto spWriter = std::make_unique<NiceMock<mock::MiddlewareWriter>>();
		        EXPECT_CALL(*spWriter, Publish(bytes)).WillOnce(Return(true));
		        return std::move(spWriter);
	        }
	    ));

	EXPECT_CALL(*m_spMockMiddlewareParticipant, CreateReader(_, _, _))
	    .WillOnce(InvokeWithoutArgs(
	        []() -> std::unique_ptr<::middleware::IMiddlewareReader>
	        {
		        auto spReader = std::make_unique<NiceMock<mock::MiddlewareReader>>();
		        EXPECT_CALL(*spReader, ConnectedToPublisher()).WillOnce(Return(true));
		        return std::move(spReader);
	        }
	    ));

	::test::MiddlewareFrameworkContext::Get().AddClient(std::move(m_spMockMiddlewareClient));
	::test::MiddlewareFrameworkContext::Get().AddServer(std::move(m_spMockMiddlewareServer));
	::test::MiddlewareFrameworkContext::Get().AddParticipant(std::move(m_spMockMiddlewareParticipant));

	auto spClient      = ::test::MiddlewareFrameworkContext::Get().TakeClient();
	auto spServer      = ::test::MiddlewareFrameworkContext::Get().TakeServer();
	auto spParticipant = ::test::MiddlewareFrameworkContext::Get().TakeParticipant();

	EXPECT_TRUE(spClient);
	EXPECT_TRUE(spServer);
	EXPECT_TRUE(spParticipant);

	auto spWriter = spParticipant->CreateWriter("MyTopic");
	auto spReader = spParticipant->CreateReader("MyTopic", nullptr, nullptr);

	EXPECT_TRUE(spReader->ConnectedToPublisher());

	std::vector<uint8_t> bytes{0x01, 0x02};
	EXPECT_TRUE(spWriter->Publish(bytes));

	::test::MiddlewareFrameworkContext::Get().Clear();
}
TEST_F(MiddlewareSystemChecks, FakeParameterServiceContextCanBeUsed)
{
	auto spMockService          = std::make_unique<NiceMock<mock::ParameterService>>();
	auto spMockPublishParameter = std::make_shared<NiceMock<mock::PublishParameter>>();
	auto spMockReadParameter    = std::make_shared<NiceMock<mock::ReadParameter>>();
	auto spMockWriteParameter   = std::make_shared<NiceMock<mock::WriteParameter>>();
	EXPECT_CALL(*spMockPublishParameter, Identity()).WillOnce(Return("Publisher"));
	EXPECT_CALL(*spMockReadParameter, Identity()).WillOnce(Return("Read"));
	EXPECT_CALL(*spMockWriteParameter, Identity()).WillOnce(Return("Write"));

	EXPECT_CALL(*spMockService, RetrievePublisherInstance("1"))
	    .WillOnce(
	        InvokeWithoutArgs([mock = spMockPublishParameter]() -> ::middleware::parameter::IPublish& { return *mock; })
	    );
	EXPECT_CALL(*spMockService, RetrieveReadInstance("2"))
	    .WillOnce(
	        InvokeWithoutArgs([mock = spMockReadParameter]() -> ::middleware::parameter::IRead& { return *mock; })
	    );
	EXPECT_CALL(*spMockService, RetrieveWriteInstance("3"))
	    .WillOnce(
	        InvokeWithoutArgs([mock = spMockWriteParameter]() -> ::middleware::parameter::IWrite& { return *mock; })
	    );

	::test::ParameterServiceContext::Get().AddParameterService(std::move(spMockService));
	auto spParameterService = ::test::ParameterServiceContext::Get().TakeParameterService();

	EXPECT_TRUE(spParameterService);

	auto& rPublisher      = spParameterService->RetrievePublisherInstance("1");
	auto& rParameterRead  = spParameterService->RetrieveReadInstance("2");
	auto& rParameterWrite = spParameterService->RetrieveWriteInstance("3");

	EXPECT_EQ(rPublisher.Identity(), "Publisher");
	EXPECT_EQ(rParameterRead.Identity(), "Read");
	EXPECT_EQ(rParameterWrite.Identity(), "Write");

	::test::ParameterServiceContext::Get().Clear();
}
