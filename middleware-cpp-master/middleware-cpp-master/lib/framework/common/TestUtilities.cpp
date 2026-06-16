// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: TestUtilities.cpp
// Description: Contains wrappers for testability.

#include "TestUtilities.h"
#include <boost/type_traits.hpp>
#include <type_traits>

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

namespace middleware
{
namespace testable
{

#define RealClock ChronoImpl::RealClock  // can't use using because the class is private
RET_TYPE(RealClock::system_clock_now) RealClock::system_clock_now()
{
	return std::chrono::system_clock::now();
}
RET_TYPE(RealClock::sleep_for) RealClock::sleep_for(ARG_TYPE(RealClock::sleep_for, 1) time)
{
	return std::this_thread::sleep_for(time);
}
RET_TYPE(RealClock::sleep_until) RealClock::sleep_until(ARG_TYPE(RealClock::sleep_until, 1) time)
{
	return std::this_thread::sleep_until(time);
}
RET_TYPE(RealClock::try_lock_for)
RealClock::try_lock_for(ARG_TYPE(RealClock::try_lock_for, 1) mutex, ARG_TYPE(RealClock::try_lock_for, 2) time)
{
	return mutex.try_lock_for(time);
}

}  // namespace testable
}  // namespace middleware
