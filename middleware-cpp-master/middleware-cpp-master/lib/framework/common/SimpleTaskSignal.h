// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: SimpleTaskSignal.h
// Description: Basic time locked task processor in a standalone thread.

#ifndef MiddlewareSimpleTaskSignal_H
#define MiddlewareSimpleTaskSignal_H

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include "SystemUtilities.h"
#include "TestUtilities.h"

namespace middleware
{
class SimpleTaskSignal
{
public:
	SimpleTaskSignal(std::function<void()>&& callback)
	{
		auto upPromise = std::make_unique<std::promise<void>>();
		m_future       = upPromise->get_future();
		m_upThread     = std::make_unique<std::thread>(
            [this, callback = std::move(callback), upPromise = std::move(upPromise)]
            {
                callback();
                upPromise->set_value();
            }
        );
	};
	~SimpleTaskSignal()
	{
		m_upThread->join();
	};
	bool Wait(std::chrono::seconds const duration = std::chrono::seconds(5))
	{
		auto status = ::middleware::testable::wait_for(m_future, duration);
		if(status == std::future_status::timeout)
		{
			::middleware::log::Warning("Timeout occurred. Terminating the thread.");
			return false;
		}
		else
		{
			return true;
		}
	};

private:
	std::unique_ptr<std::thread> m_upThread;
	std::future<void> m_future;
};
}  // namespace middleware

#endif  // MiddlewareSimpleTaskSignal_H
