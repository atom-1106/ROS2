// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ParameterServiceOwnerInstance.cpp
// Description: Integration Owner Instance Handler Helper Class

#include "ParameterServiceOwnerInstance.h"

#include <gtest/gtest.h>

ParameterServiceOwnerInstance::ParameterServiceOwnerInstance(std::string const& name)
    : m_spConfiguration{middleware::parameter::IConfigBuilder::Create(std::string{name})}
{
}
ParameterServiceOwnerInstance::~ParameterServiceOwnerInstance()
{
	Stop();
}
void ParameterServiceOwnerInstance::Stop()
{
	{
		std::unique_lock<std::mutex> lock{m_mutex};
		m_run.store(false);
		m_wakeup.store(true);
	}
	m_condition.notify_all();
	if(m_thread.joinable())
	{
		m_thread.join();
	}
}
void ParameterServiceOwnerInstance::AddPublisher(
    std::string const& name,
    std::string const& initialValue,
    ::cat::middleware::parameter::Quality const initialQuality
)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	middleware::parameter::Data initialData{};
	initialData.mutable_value()->set_string_value(initialValue);
	initialData.set_quality(initialQuality);
	m_spConfiguration->AddPublish(std::string{name}, {}, std::move(initialData));
}
void ParameterServiceOwnerInstance::AddPublisher(
    std::string const& name,
    std::string const& initialValue,
    ::middleware::parameter::Unit const unit,
    ::cat::middleware::parameter::Quality const initialQuality
)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	middleware::parameter::Data initialData{};
	initialData.mutable_value()->set_string_value(initialValue);
	initialData.set_quality(initialQuality);
	m_spConfiguration->AddPublish(std::string{name}, unit, std::move(initialData));
}
void ParameterServiceOwnerInstance::AddOnWrite(
    std::string const& name,
    ::cat::middleware::parameter::Quality const returnQuality
)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto on_write = [quality = returnQuality](auto, auto const& id, auto const& data)
	{
		::middleware::parameter::Data data_written{};
		data_written.CopyFrom(data);
		data_written.set_quality(quality);
		return data_written;
	};
	m_spConfiguration->AddServerWrite(std::string{name}, on_write);
}
void ParameterServiceOwnerInstance::AddOnRead(
    std::string const& name,
    std::string const& returnValue,
    ::cat::middleware::parameter::Quality const returnQuality
)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto on_read = [value = returnValue, quality = returnQuality](auto, auto const& id)
	{
		::middleware::parameter::Data data_read{};
		data_read.mutable_value()->set_string_value(value);
		data_read.set_quality(quality);
		return data_read;
	};
	m_spConfiguration->AddServerRead(std::string{name}, on_read);
}
void ParameterServiceOwnerInstance::NextValue(
    std::string const& name,
    std::string const& value,
    ::cat::middleware::parameter::Quality const quality
)
{
	std::unique_lock<std::mutex> lock{m_mutex};
	// Add to queue
	NextValueType next{};
	next.name = name;
	next.data.mutable_value()->set_string_value(value);
	next.data.set_quality(quality);
	m_nextValueToPublishQueue.push(std::move(next));
	// wakeup
	m_wakeup.store(true);
	lock.unlock();
	m_condition.notify_one();
}

void ParameterServiceOwnerInstance::Run()
{
	ASSERT_FALSE(m_run) << "Owner Instance thread is already running!!!";

	m_thread = std::thread(
	    [this]
	    {
		    m_run.store(true);
		    auto spOwnerServiceInstance =
		        ::middleware::parameter::Factory::CreateParameterService(std::move(m_spConfiguration));

		    while(m_run)
		    {
			    std::unique_lock<std::mutex> lock{m_mutex};
			    m_condition.wait(lock, [this] { return m_wakeup.exchange(false); });
			    if(!m_run)
			    {
				    break;
			    }
			    while(!m_nextValueToPublishQueue.empty())
			    {
				    auto next = m_nextValueToPublishQueue.front();
				    m_nextValueToPublishQueue.pop();

				    auto& parameter = spOwnerServiceInstance->RetrievePublisherInstance(next.name);
				    parameter.Publish(next.data);
			    }
		    }
	    }
	);
}
