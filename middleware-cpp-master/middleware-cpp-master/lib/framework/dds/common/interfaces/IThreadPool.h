// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: IThreadPool.h
// Description: Interface for wrapper around ::boost::asio::thread_pool.

#ifndef IMiddlewareThreadPool_H
#define IMiddlewareThreadPool_H

#include <functional>

namespace middleware
{
class IThreadPool
{
public:
	using Task             = std::function<void()>;
	virtual ~IThreadPool() = default;
	// Adds a thread task to the thread pool.
	virtual void Add(Task&& task) = 0;
	// Stops the ability to add more thread tasks to the thread pool
	// and joins the threads back to the main thread.
	virtual void Stop() = 0;
};

}  // namespace middleware

#endif  // IMiddlewareThreadPool_H
