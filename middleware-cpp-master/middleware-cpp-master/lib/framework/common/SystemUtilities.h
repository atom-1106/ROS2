// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: SystemUtilities.h
// Description: Contains the system utilities for middleware.

#ifndef SystemUtilities_H
#define SystemUtilities_H

#include <google/protobuf/message.h>

#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

namespace middleware
{
class common
{
public:
	static uint64_t TimeNowAsMs();
	static std::string TimeNowAsFormat();
	static std::vector<uint8_t> SerializeToVector(::google::protobuf::Message const& message);
	static void SerializeToVector(::google::protobuf::Message const& message, std::vector<uint8_t>& data);
};
class log
{
public:
	enum class LogLevel
	{
		LEVEL_ONE,    // Errors
		LEVEL_TWO,    // Warnings + LEVEL_ONE
		LEVEL_THREE,  // Additional Information + LEVEL_TWO
		LEVEL_FOUR,   // Debug Information + LEVEL_THREE
	};
	static void Initialize(std::ostream& out, LogLevel const lvl = LogLevel::LEVEL_TWO);
	static void Reset();
	static void Debug(std::string const& format, ...);
	static void Info(std::string const& format, ...);
	static void Warning(std::string const& format, ...);
	// Note: Will throw an exception
	static void Error(std::string const& format, ...);

private:
};
}  // namespace middleware

#endif  // SystemUtilities_H
