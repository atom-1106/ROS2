// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: TimestampedValue.h
// Description: Contains timestamp wrapper.

#ifndef TimestampedValue_H
#define TimestampedValue_H

#include <chrono>

namespace middleware
{

template <typename E, typename C = std::chrono::system_clock>
struct TimestampedValue
{
	E value;
	std::chrono::time_point<C> timestamp;
};

}  // namespace middleware

#endif  // TimestampedValue_H
