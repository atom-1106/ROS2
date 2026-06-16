// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ThreadPool.h
// Description: Implementation wrapper around ::boost::asio::thread_pool.

#ifndef MiddlewareThreadPool_H
#define MiddlewareThreadPool_H

#include "IThreadPool.h"

#include <boost/asio/thread_pool.hpp>
#include <cstdint>

namespace middleware
{
class ThreadPool final : public ::middleware::IThreadPool
{
public:
	ThreadPool(uint32_t thread_pool_size);
	virtual ~ThreadPool() override = default;
	void Add(::middleware::IThreadPool::Task&& task) override;
	void Stop() override;

private:
	::boost::asio::thread_pool m_ThreadPool;
};

}  // namespace middleware

#endif  // MiddlewareThreadPool_H
