// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ThreadPool.cpp
// Description: Implementation wrapper around ::boost::asio::thread_pool.

#include "ThreadPool.h"

#include <boost/asio/post.hpp>

namespace middleware
{
ThreadPool::ThreadPool(uint32_t thread_pool_size) : m_ThreadPool{thread_pool_size}
{
}
void ThreadPool::Add(::middleware::IThreadPool::Task&& task)
{
	::boost::asio::post(m_ThreadPool, std::move(task));
}
void ThreadPool::Stop()
{
	m_ThreadPool.stop();
}
}  // namespace middleware
