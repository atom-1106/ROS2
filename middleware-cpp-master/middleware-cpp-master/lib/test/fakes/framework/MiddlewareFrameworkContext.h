// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareFrameworkContext.h
// Description: Context control for FakeMiddlewareFrameworkFactory implementation.

#ifndef MiddlewareFrameworkContext_H
#define MiddlewareFrameworkContext_H

#include "mockMiddlewareClient.h"
#include "mockMiddlewareParticipant.h"
#include "mockMiddlewareServer.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <mutex>
#include <queue>

using namespace ::testing;

namespace test
{
class MiddlewareFrameworkContext
{
private:
	MiddlewareFrameworkContext() {};

public:
	virtual ~MiddlewareFrameworkContext() = default;

public:
	static MiddlewareFrameworkContext& Get()
	{
		static MiddlewareFrameworkContext instance{};
		return instance;
	}
	void AddClient(std::unique_ptr<mock::MiddlewareClient>&& spMock)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_clientQueue.push(std::move(spMock));
	}
	void AddServer(std::unique_ptr<mock::MiddlewareServer>&& spMock)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_serverQueue.push(std::move(spMock));
	}
	void AddParticipant(std::unique_ptr<mock::MiddlewareParticipant>&& spMock)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_participantQueue.push(std::move(spMock));
	}

	std::unique_ptr<mock::MiddlewareClient> TakeClient()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if(m_clientQueue.empty())
		{
			return nullptr;
		}
		auto out = std::move(m_clientQueue.front());
		m_clientQueue.pop();
		return out;
	}
	std::unique_ptr<mock::MiddlewareServer> TakeServer()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if(m_serverQueue.empty())
		{
			return nullptr;
		}
		auto out = std::move(m_serverQueue.front());
		m_serverQueue.pop();
		return out;
	}
	std::unique_ptr<mock::MiddlewareParticipant> TakeParticipant()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if(m_participantQueue.empty())
		{
			return nullptr;
		}
		auto out = std::move(m_participantQueue.front());
		m_participantQueue.pop();
		return out;
	}
	void Clear()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		while(!m_clientQueue.empty())
		{
			m_clientQueue.pop();
		}
		while(!m_serverQueue.empty())
		{
			m_serverQueue.pop();
		}
		while(!m_participantQueue.empty())
		{
			m_participantQueue.pop();
		}
	}

private:
	std::mutex m_mutex;
	std::queue<std::unique_ptr<mock::MiddlewareClient>> m_clientQueue{};
	std::queue<std::unique_ptr<mock::MiddlewareServer>> m_serverQueue{};
	std::queue<std::unique_ptr<mock::MiddlewareParticipant>> m_participantQueue{};
};
}  // namespace test

#endif  // MiddlewareFrameworkContext_H
