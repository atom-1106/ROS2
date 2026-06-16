// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ParameterServiceContext.h
// Description: Context control for FakeParameterServiceFactory implementation.

#ifndef ParameterServiceContext_H
#define ParameterServiceContext_H

#include "mockParameterService.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mutex>
#include <queue>

using namespace ::testing;

namespace test
{
class ParameterServiceContext
{
private:
	ParameterServiceContext() {};

public:
	virtual ~ParameterServiceContext() = default;
	static ParameterServiceContext& Get()
	{
		static ParameterServiceContext instance{};
		return instance;
	}
	void AddParameterService(std::unique_ptr<mock::ParameterService>&& spMock)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_parameterService.push(std::move(spMock));
	}
	std::unique_ptr<mock::ParameterService> TakeParameterService()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if(m_parameterService.empty())
		{
			return nullptr;
		}
		auto out = std::move(m_parameterService.front());
		m_parameterService.pop();
		return out;
	}
	void Clear()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		while(!m_parameterService.empty())
		{
			m_parameterService.pop();
		}
	}

private:
	std::mutex m_mutex;
	std::queue<std::unique_ptr<mock::ParameterService>> m_parameterService{};
};
}  // namespace test

#endif  // ParameterServiceContext_H
