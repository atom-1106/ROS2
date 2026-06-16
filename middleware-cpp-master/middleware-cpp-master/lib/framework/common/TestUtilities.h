// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: TestUtilities.h
// Description: Contains wrappers for testability.

#ifndef TestUtilities_H
#define TestUtilities_H

#include <chrono>
#include <future>
#include <memory>
#include <mutex>
#include <semaphore>
#include <thread>
#include <type_traits>

// nanoseconds types
#define NS(x) std::chrono::duration_cast<std::chrono::nanoseconds>(x)
#define NS_TYPE std::chrono::nanoseconds
#define NS_VAL std::declval<std::chrono::nanoseconds>()
// system clock types
#define SC(x) std::chrono::time_point_cast<std::chrono::system_clock>(x)
#define SC_TYPE std::chrono::time_point<std::chrono::system_clock>
#define SC_VAL std::declval<std::chrono::time_point<std::chrono::system_clock>>()

namespace middleware
{
namespace testable
{

// ChronoImpl has a singleton get()/set() for itself which defaults to RealClock
// it only implements nanosecond and system_clock functions
// "namespaced" static functions take any allowed type and convert to ^ to throw into the ChronoImpl singleton
// RealClock is a private implementation of ChronoImpl, pointing to the usual static methods
// a test implementation can be used instead at runtime with set()
class ChronoImpl
{
public:
	ChronoImpl() : elapsed{0}, ref{std::chrono::system_clock::now()} {};
	//virtual ~ChronoImpl() = 0; // causes issues because unique_ptr destructs from this superclass
	static inline ChronoImpl* const get();
	static void set(std::unique_ptr<ChronoImpl>&& newInstance)
	{
		m_spChronoInstance = std::move(newInstance);
	};
	static inline void reset();

	virtual decltype(std::chrono::system_clock::now()) system_clock_now()               = 0;
	virtual decltype(std::this_thread::sleep_for(NS_VAL)) sleep_for(NS_TYPE&& time)     = 0;
	virtual decltype(std::this_thread::sleep_until(SC_VAL)) sleep_until(SC_TYPE&& time) = 0;
	virtual bool try_lock_for(std::timed_mutex& mutex, NS_TYPE&& time)                  = 0;

	class RealClock;
	static inline uint8_t const test_poll_ms = 10;
	std::chrono::milliseconds elapsed;
	std::chrono::time_point<std::chrono::system_clock> ref;
	std::binary_semaphore time_event{0};

private:
	static std::unique_ptr<ChronoImpl> m_spChronoInstance;
};
class ChronoImpl::RealClock : public ChronoImpl
{
public:
	decltype(std::chrono::system_clock::now()) system_clock_now() override;
	decltype(std::this_thread::sleep_for(NS_VAL)) sleep_for(NS_TYPE&& time) override;
	decltype(std::this_thread::sleep_until(SC_VAL)) sleep_until(SC_TYPE&& time) override;
	bool try_lock_for(std::timed_mutex& mutex, NS_TYPE&& time) override;
};
inline ChronoImpl* const ChronoImpl::get()
{
	if(!m_spChronoInstance)
	{
		m_spChronoInstance = std::make_unique<ChronoImpl::RealClock>();
	}
	return m_spChronoInstance.get();
}
inline void ChronoImpl::reset()
{
	m_spChronoInstance = std::make_unique<ChronoImpl::RealClock>();
}

// methods for user use
struct system_clock
{
	static auto now()
	{
		return ChronoImpl::get()->system_clock_now();
	}
};
struct this_thread
{
	template <typename T>
	static auto sleep_for(T&& time)
	{
		return ChronoImpl::get()->sleep_for(NS(time));
	}
	template <typename T>
	static auto sleep_until(T&& time)
	{
		return ChronoImpl::get()->sleep_until(SC(time));
	}
};
static inline bool try_lock_for(std::timed_mutex& mutex, NS_TYPE&& time)
{
	return ChronoImpl::get()->try_lock_for(mutex, NS(time));
}
// have to do this here because counting_semaphore is generic
// this is also why more things are public :(
template <std::ptrdiff_t N>
static inline bool try_acquire_for(std::counting_semaphore<N>& sem, NS_TYPE&& time)
{
	ChronoImpl* impl = ChronoImpl::get();
	if(dynamic_cast<ChronoImpl::RealClock*>(impl) != nullptr)
	{
		return sem.try_acquire_for(time);
	}
	else
	{
		impl->time_event.release();
		std::chrono::time_point<std::chrono::system_clock> start = impl->ref + impl->elapsed;
		while(!sem.try_acquire_for(std::chrono::milliseconds(ChronoImpl::test_poll_ms)))
		{
			if(impl->ref + impl->elapsed >= start + time)
			{
				impl->time_event.release();
				return false;
			}
		}
		return true;
	}
}
template <typename P>
static inline std::future_status wait_for(std::future<P>& future, NS_TYPE&& time)
{
	ChronoImpl* impl = ChronoImpl::get();
	if(dynamic_cast<ChronoImpl::RealClock*>(impl) != nullptr)
	{
		return future.wait_for(time);
	}
	else
	{
		impl->time_event.release();
		std::chrono::time_point<std::chrono::system_clock> start = impl->ref + impl->elapsed;
		std::future_status ret;
		while((ret = future.wait_for(std::chrono::milliseconds(ChronoImpl::test_poll_ms))) ==
		      std::future_status::timeout)
		{
			if(impl->ref + impl->elapsed >= start + time)
			{
				impl->time_event.release();
				return std::future_status::timeout;
			}
		}
		// necessary for running in gdb where there could be random delays
		if(impl->ref + impl->elapsed >= start + time)
		{
			impl->time_event.release();
			return std::future_status::timeout;
		}
		return ret;
	}
}

/// Allows access to the object passed in even after the unique_ptr has been moved away.
/// However do not access this if the unique_ptr is taken and then destroyed! Whatever
/// owns the taken unique_ptr must have a lifetime at least as long as this object.
template <typename T>
class TestUniquePtr
{
public:
	TestUniquePtr() = default;
	TestUniquePtr(std::unique_ptr<T>&& spT) : m_spTransientT(std::move(spT)), m_pT(m_spTransientT.get())
	{
		assert(m_spTransientT);
	}
	TestUniquePtr& operator=(std::unique_ptr<T>&& spT)
	{
		assert(spT);
		m_spTransientT = std::move(spT);
		m_pT           = m_spTransientT.get();

		return *this;
	}

	std::unique_ptr<T>&& TakeUniquePtr()
	{
		assert(!Taken());
		return std::move(m_spTransientT);
	}
	T& operator*()
	{
		return *m_pT;
	}
	T* operator->() const
	{
		assert(m_pT != nullptr);
		return m_pT;
	}
	T* get() const
	{
		return m_pT;
	}
	void reset()
	{
		m_spTransientT.reset();
		m_pT = nullptr;
	}
	bool Taken() const
	{
		return m_pT && !m_spTransientT;
	}

private:
	std::unique_ptr<T> m_spTransientT{};  // Can be moved out of
	T* m_pT{};
};

template <typename T>
class DefaultTestUniquePtr : public TestUniquePtr<T>
{
public:
	DefaultTestUniquePtr() : TestUniquePtr<T>(std::make_unique<T>())
	{
	}
	void Remake()
	{
		*this = DefaultTestUniquePtr{};
	}
};

template <typename T, typename... Args>
TestUniquePtr<T> MakeTestUniquePtr(Args&&... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}
template <typename T>
TestUniquePtr<T> MakeTestUniquePtr(std::unique_ptr<T>&& spT)
{
	return std::move(spT);
}

}  // namespace testable
}  // namespace middleware

#undef NS
#undef NS_TYPE
#undef NS_VAL
#undef SC
#undef SC_TYPE
#undef SC_VAL

#endif  // TestUtilities_H
