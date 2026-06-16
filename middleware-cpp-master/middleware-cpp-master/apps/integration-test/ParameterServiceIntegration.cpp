// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ParameterServiceIntegration.cpp
// Description: Integration Tests for Middleware Parameter Service

#include "ParameterServiceOwnerInstance.h"

#include <IBasicParameter.h>
#include <IConfigBuilder.h>
#include <IConfigTaker.h>
#include <IParameterService.h>
#include <IPublishParameter.h>
#include <IReadParameter.h>
#include <IWriteParameter.h>
#include <ParameterServiceDefines.h>
#include <ParameterServiceFactory.h>
#include <middleware_parameter.pb.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <chrono>
#include <condition_variable>
#include <optional>
#include <string>
#include <thread>
#include <vector>

using namespace ::testing;
namespace
{
constexpr char n_IntegrationClientName[] = "IntegrationClient";
constexpr char n_IntegrationServerName[] = "IntegrationServer";

// System Byte Size Max allowed to be published
constexpr uint32_t n_MaxAllowedDataSize = 4000000;  // 4 MB Limit - protobuf metadata;
constexpr auto n_ClientTimeoutInterval  = std::chrono::seconds(10);
}  // namespace

class ParameterServiceIntegration : public ::testing::Test
{
protected:
	ParameterServiceIntegration()
	    : m_spClientConfiguration{middleware::parameter::IConfigBuilder::Create(n_IntegrationClientName)}
	{
	}
	virtual ~ParameterServiceIntegration() = default;
	void AddSubscription(std::string const& name)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_receivedSubscriptionData[name] = std::nullopt;

		m_spClientConfiguration->AddSubscribe(
		    std::string{name},
		    {},
		    [this](auto const& id, auto const& data) { ProcessReceivedSubscription(id, data); },
		    []() {}
		);
	}
	void AddSubscriptionWithMetadata(std::string const& name)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_receivedSubscriptionData[name] = std::nullopt;

		m_spClientConfiguration->AddSubscribe(
		    std::string{name},
		    [this](auto const& id, auto const& data, auto const& meta) { ProcessReceivedSubscription(id, data, meta); },
		    []() {}
		);
	}
	void AddClientRead(std::string const& name)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_spClientConfiguration->AddClientRead(std::string{name}, {});
	}
	void AddClientWrite(std::string const& name)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_spClientConfiguration->AddClientWrite(std::string{name}, {});
	}
	void ProcessReceivedSubscription(
	    std::string const& name,
	    ::middleware::parameter::Data const& data,
	    ::middleware::parameter::Metadata meta = {}
	)
	{
		std::unique_lock<std::mutex> lock{m_mutex};
		ASSERT_THAT(m_receivedSubscriptionData, Contains(Key(name)));
		auto found = m_receivedSubscriptionData.find(name);
		if(!found->second.has_value())
		{
			found->second = {::middleware::parameter::Data{}, ::middleware::parameter::Metadata{}};
			found->second.value().data.CopyFrom(data);
			found->second.value().metadata.unit = meta.unit;
		}
		else
		{
			found->second.reset();
			FAIL() << "Parameter: [" << name << "] value was pending. Unexpected value published!!";
		}

		m_received.store(true);
		lock.unlock();
		m_condition.notify_one();
	}
	void VerifyAndClearSubscriptionData(
	    std::string const& name,
	    std::string const& value,
	    ::cat::middleware::parameter::Quality const quality,
	    ::middleware::parameter::Metadata meta = {}
	)
	{
		std::lock_guard<std::mutex> lock{m_mutex};
		ASSERT_THAT(m_receivedSubscriptionData, Contains(Key(name)));
		auto found = m_receivedSubscriptionData.find(name);
		ASSERT_TRUE(found->second.has_value());
		EXPECT_THAT(found->second.value().data.value().string_value(), Eq(value));
		EXPECT_THAT(found->second.value().data.quality(), Eq(quality));
		EXPECT_THAT(found->second.value().metadata.unit, Eq(meta.unit));
		found->second.reset();
	}
	bool WaitForData(std::chrono::milliseconds interval = n_ClientTimeoutInterval)
	{
		std::unique_lock<std::mutex> lock{m_mutex};
		return m_condition.wait_for(lock, interval, [this] { return m_received.exchange(false); });
	}
	std::string ClampValueToLength(
	    std::string const& name,
	    std::string const& value,
	    ::cat::middleware::parameter::Quality const quality
	)
	{
		// Note: Clamping is required for large boundary values due to protobuf metadata bytes being
		//		 added during serialization which will not get the length to the expectation.
		::cat::middleware::parameter::Parameter parameter{};
		parameter.mutable_info()->set_identity(name);
		parameter.mutable_data()->mutable_value()->set_string_value(value);
		parameter.mutable_data()->set_quality(quality);

		auto totalSize = parameter.SerializeAsString().size();
		if(totalSize > n_MaxAllowedDataSize)
		{
			auto offset = totalSize - value.size();
			std::string trimmed{value};
			trimmed.resize(value.size() - offset);
			parameter.mutable_data()->mutable_value()->set_string_value(trimmed);
			return trimmed;
		}
		return value;
	}
	struct DataCapture
	{
		::middleware::parameter::Data data{};
		::middleware::parameter::Metadata metadata{};
	};
	using LastKnownValue = std::optional<DataCapture>;
	std::unique_ptr<::middleware::parameter::IConfigBuilder> m_spClientConfiguration;

	std::map<std::string, LastKnownValue> m_receivedSubscriptionData{};
	std::mutex m_mutex{};
	std::atomic_bool m_received{false};
	std::condition_variable m_condition{};
};
TEST_F(ParameterServiceIntegration, DataLargerThan4MbWillNotCallSubscriptionWhenPublished)
{
	std::string parameterName{"MyParameter"};
	std::string oneByte(255, 'A');

	// Will not be published
	std::string beyondMaxValue(n_MaxAllowedDataSize + 1, 'D');

	AddSubscription(parameterName);
	auto spClientServiceInstance =
	    ::middleware::parameter::Factory::CreateParameterService(std::move(m_spClientConfiguration));

	auto spOwnerInstance = std::make_unique<ParameterServiceOwnerInstance>(n_IntegrationServerName);
	spOwnerInstance->AddPublisher(parameterName, oneByte, ::cat::middleware::parameter::QUALITY_GOOD);
	spOwnerInstance->Run();

	// Initial Publish, small 255 bytes
	WaitForData();
	VerifyAndClearSubscriptionData(parameterName, oneByte, ::cat::middleware::parameter::QUALITY_GOOD);

	// Publish (4 MB + 1 byte) -> Last value will remain unchanged but wait a little to make sure
	auto clampedValue = ClampValueToLength(parameterName, beyondMaxValue, ::cat::middleware::parameter::QUALITY_GOOD);
	spOwnerInstance->NextValue(parameterName, clampedValue, ::cat::middleware::parameter::QUALITY_GOOD);
	auto status = WaitForData(std::chrono::milliseconds{100});
	EXPECT_FALSE(status);

	// Stop Instance thread
	spOwnerInstance->Stop();
}

TEST_F(ParameterServiceIntegration, PublisherSetsParameterDataQualityBadWhenDisconnected)
{
	std::string parameterName{"Fruit"};
	std::string value{"Apple"};

	AddSubscription(parameterName);
	auto spClientServiceInstance =
	    ::middleware::parameter::Factory::CreateParameterService(std::move(m_spClientConfiguration));

	auto spOwnerInstance = std::make_unique<ParameterServiceOwnerInstance>(n_IntegrationServerName);
	spOwnerInstance->AddPublisher(parameterName, value, ::cat::middleware::parameter::QUALITY_GOOD);
	spOwnerInstance->Run();

	// Initial Publish, small 255 bytes
	WaitForData();
	VerifyAndClearSubscriptionData(parameterName, value, ::cat::middleware::parameter::QUALITY_GOOD);

	// Stop Instance thread
	spOwnerInstance->Stop();

	// Owner service disconnected
	WaitForData();
	VerifyAndClearSubscriptionData(parameterName, value, ::cat::middleware::parameter::QUALITY_BAD);
}
TEST_F(ParameterServiceIntegration, PublisherSendsMetadataToSubscribersWhenCallbackIsConfigured)
{
	std::string parameterName{"Fruit"};
	std::string value{"Apple"};

	AddSubscriptionWithMetadata(parameterName);
	auto spClientServiceInstance =
	    ::middleware::parameter::Factory::CreateParameterService(std::move(m_spClientConfiguration));

	auto spOwnerInstance = std::make_unique<ParameterServiceOwnerInstance>(n_IntegrationServerName);
	spOwnerInstance->AddPublisher(
	    parameterName, value, ::middleware::parameter::Unit::gal, ::cat::middleware::parameter::QUALITY_GOOD
	);
	spOwnerInstance->Run();

	// Initial Publish, small 255 bytes
	::middleware::parameter::Metadata expectedMetadata{::middleware::parameter::Unit::gal};
	WaitForData();
	VerifyAndClearSubscriptionData(parameterName, value, ::cat::middleware::parameter::QUALITY_GOOD, expectedMetadata);

	// Stop Instance thread
	spOwnerInstance->Stop();

	// Owner service disconnected
	WaitForData();
	VerifyAndClearSubscriptionData(parameterName, value, ::cat::middleware::parameter::QUALITY_BAD, expectedMetadata);
}
namespace
{
struct PublishTestData
{
	std::string name{};
	std::string value{};
	static std::string TestName(testing::TestParamInfo<PublishTestData> const& data)
	{
		return data.param.name;
	}
};
PublishTestData const n_TestDataForPublish[] = {
    {"255B", std::string(255, 'A')},
    {"10KB", std::string(10000, 'B')},
    {"500KB", std::string(500000, 'B')},
    {"1MB", std::string(1000000, 'C')},
    {"4MB", std::string(n_MaxAllowedDataSize, 'D')}  // System max is 4MB but protobuf metadata consumes space
};
}  // namespace

class PublishVariedSizes : public ParameterServiceIntegration, public WithParamInterface<PublishTestData>
{
};
INSTANTIATE_TEST_SUITE_P(
    ParameterServiceIntegration,
    PublishVariedSizes,
    ValuesIn(n_TestDataForPublish),
    PublishTestData::TestName
);

TEST_P(PublishVariedSizes, DataWithinBoundsWillCallSubscriptionWhenPublished)
{
	std::string parameterName{"MyParameter"};
	std::string initialValue{"default"};

	AddSubscription(parameterName);
	auto spClientServiceInstance =
	    ::middleware::parameter::Factory::CreateParameterService(std::move(m_spClientConfiguration));

	auto spOwnerInstance = std::make_unique<ParameterServiceOwnerInstance>(n_IntegrationServerName);
	spOwnerInstance->AddPublisher(parameterName, initialValue, ::cat::middleware::parameter::QUALITY_GOOD);
	spOwnerInstance->Run();

	// Verify Setup
	WaitForData();
	VerifyAndClearSubscriptionData(parameterName, initialValue, ::cat::middleware::parameter::QUALITY_GOOD);

	// Test
	auto clampedValue = ClampValueToLength(parameterName, GetParam().value, ::cat::middleware::parameter::QUALITY_GOOD);
	spOwnerInstance->NextValue(parameterName, clampedValue, ::cat::middleware::parameter::QUALITY_GOOD);
	WaitForData();
	VerifyAndClearSubscriptionData(parameterName, clampedValue, ::cat::middleware::parameter::QUALITY_GOOD);

	// Stop Instance thread
	spOwnerInstance->Stop();
}
