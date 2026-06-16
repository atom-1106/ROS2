// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: FakeTestUtilities.h
// Description: Contains fakes for middleware testability wrappers.

#ifndef FakeTestUtilities_H
#define FakeTestUtilities_H

#include <boost/type_traits.hpp>
#include <chrono>
#include <thread>
#include <type_traits>
#include "TestUtilities.h"

// type of member function pointer -> type of function
template <typename T>
struct remove_this;
template <typename R, typename C, typename... A>
struct remove_this<R (C::*)(A...)>
{
	using type = R(A...);
};
#define BOOST_FUNC_TRAITS(f) boost::function_traits<remove_this<decltype(&f)>::type>
#define RET_TYPE(f) BOOST_FUNC_TRAITS(f)::result_type
#define BOOST_NTH_ARG(x) arg##x##_type
#define ARG_TYPE(f, x) BOOST_FUNC_TRAITS(f)::BOOST_NTH_ARG(x)

namespace
{
using namespace ::middleware::testable;

class FakeClock : public ChronoImpl
{
public:
	class FakeClockLifetime
	{
		FakeClock* clock;

	public:
		FakeClockLifetime()
		{
			auto newClock = std::make_unique<FakeClock>();
			clock         = newClock.get();
			ChronoImpl::set(std::move(newClock));
		};
		~FakeClockLifetime()
		{
			ChronoImpl::reset();
		};
		FakeClock* operator->()
		{
			return clock;
		};
	};
	// cppcheck-suppress duplInheritedMember
	static std::unique_ptr<FakeClockLifetime> get()
	{
		return std::make_unique<FakeClockLifetime>();
	};
	// ensure that no time events happen for N consecutive milliseconds
	// other threads should get stuck in their time methods, making this eventually progress
	// system_clock_now does not flush this in case of timing loops, logging, etc.
	void flush_time(size_t milliseconds)
	{
		if(milliseconds == 0)
		{
			std::this_thread::yield();
			std::this_thread::yield();
		}
		else
		{
			if(milliseconds < test_poll_ms)
				milliseconds = test_poll_ms;
			(void)time_event.try_acquire();
			while(time_event.try_acquire_for(std::chrono::milliseconds(milliseconds)))
			{
			};
		}
	};
	// should be used to ensure things waiting have been released if necessary
	inline void flush_time()
	{
		flush_time(test_poll_ms);  // has to be >= test_poll_ms
		flush_time(0);
	};
	void advance_ms(size_t milliseconds)
	{
		flush_time(0);
		elapsed += std::chrono::milliseconds(milliseconds);
	};

	RET_TYPE(ChronoImpl::system_clock_now) system_clock_now() override
	{
		return ref + elapsed;
	};
	RET_TYPE(ChronoImpl::sleep_for) sleep_for(ARG_TYPE(ChronoImpl::sleep_for, 1) time) override
	{
		time_event.release();
		std::chrono::time_point<std::chrono::system_clock> start = ref + elapsed;
		while(ref + elapsed < start + time)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(test_poll_ms));
		}
	};
	RET_TYPE(ChronoImpl::sleep_until) sleep_until(ARG_TYPE(ChronoImpl::sleep_until, 1) time) override
	{
		return sleep_for(time - (ref + elapsed));
	};
	RET_TYPE(ChronoImpl::try_lock_for)
	try_lock_for(ARG_TYPE(ChronoImpl::try_lock_for, 1) mutex, ARG_TYPE(ChronoImpl::try_lock_for, 2) time) override
	{
		time_event.release();
		std::chrono::time_point<std::chrono::system_clock> start = ref + elapsed;
		while(!mutex.try_lock_for(std::chrono::milliseconds(test_poll_ms)))
		{
			if(ref + elapsed >= start + time)
			{
				time_event.release();
				return false;
			}
		}
		return true;
	};
};

}  // namespace

#undef BOOST_FUNC_TRAITS
#undef RET_TYPE
#undef BOOST_NTH_ARG
#undef ARG_TYPE

#endif  // FakeTestUtilities_H
