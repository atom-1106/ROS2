// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: SingleThreadedUsageValidator.h
// Description: Single Threaded Usage Validator

#ifndef SingleThreadedUsageValidator_H
#define SingleThreadedUsageValidator_H

#include <memory>
#include <thread>

namespace middleware
{
class SingleThreadedUsageValidator final
{
#ifndef NDEBUG
public:
	/// May be overridden by a non-weak symbol. (Useful for Acceptance Tests where
	/// we may be spawning a new main thread for each test.)
	void ValidateCurrentThread() const;

private:
	static std::thread::id GetId() noexcept
	{
		return std::this_thread::get_id();
	}

	std::thread::id const m_threadId{GetId()};
#else
public:
	inline void ValidateCurrentThread() const
	{
	}
#endif
};

class LazySingleThreadedUsageValidator final
{
#ifndef NDEBUG
public:
	void ValidateCurrentThread() const;

private:
	mutable std::unique_ptr<SingleThreadedUsageValidator> m_spSingleThreadedUsageValidator{};
#else
public:
	inline void ValidateCurrentThread() const
	{
	}
#endif
};
}  // namespace middleware
#endif  // SingleThreadedUsageValidator_H
