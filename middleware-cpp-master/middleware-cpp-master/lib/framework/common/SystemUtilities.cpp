// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: SystemUtilities.cpp
// Description: Contains the system utilities for middleware.

#include "SystemUtilities.h"
#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <iostream>
#include "TestUtilities.h"
// Boost
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>

// BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", boost::log::trivial::severity_level)
// BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)

#define C_DEF "\033[m"
#define C_RED "\033[31m"
#define C_B_RED "\033[31;1m"
#define C_GREEN "\033[32m"
#define C_B_GREEN "\033[32;1m"
#define C_YELLOW "\033[33m"
#define C_B_YELLOW "\033[33;1m"
#define C_BLUE "\033[34m"
#define C_B_BLUE "\033[34;1m"
#define C_MAGENTA "\033[35m"
#define C_B_MAGENTA "\033[35;1m"
#define C_CYAN "\033[36m"
#define C_B_CYAN "\033[36;1m"
#define C_WHITE "\033[37m"
#define C_B_WHITE "\033[37;1m"
#define C_BRIGHT "\033[1m"

namespace
{
static constexpr char n_TimestampFormat[]                = "%Y-%m-%d %H:%M:%S";
static boost::shared_ptr<boost::log::core> n_LogSettings = nullptr;
static middleware::log::LogLevel n_activeLevel           = middleware::log::LogLevel::LEVEL_TWO;

}  // namespace

///////////////////////////////////
namespace middleware
{
uint64_t common::TimeNowAsMs()
{
	// Get current time
	auto now      = ::middleware::testable::system_clock::now();
	auto duration = now.time_since_epoch();
	return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}
std::string common::TimeNowAsFormat()
{
	// Get current time
	auto now        = ::middleware::testable::system_clock::now();
	auto time_t_now = std::chrono::system_clock::to_time_t(now);

	// Convert to tm struct for local time
	std::tm tm_now;
	localtime_r(&time_t_now, &tm_now);

	// Format date and time
	std::ostringstream oss;
	oss << std::put_time(&tm_now, n_TimestampFormat);

	// Add milliseconds
	auto duration     = now.time_since_epoch();
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;
	oss << ':' << std::setfill('0') << std::setw(3) << milliseconds;

	return oss.str();
}
std::vector<uint8_t> common::SerializeToVector(::google::protobuf::Message const& message)
{
	std::vector<uint8_t> data(message.ByteSizeLong());
	[[maybe_unused]] bool result = message.SerializeToArray(data.data(), data.size());
	assert(result);
	return data;
}
void common::SerializeToVector(::google::protobuf::Message const& message, std::vector<uint8_t>& data)
{
	data.resize(message.ByteSizeLong());
	[[maybe_unused]] bool result = message.SerializeToArray(data.data(), data.size());
	assert(result);
}
void log::Initialize(std::ostream& out, log::LogLevel const lvl)
{
	if(n_LogSettings)
	{
		::middleware::log::Warning("Middleware Logger already initialized!");
		return;
	}
	n_LogSettings = boost::log::core::get();

	// Add a sink that writes to the stringstream
	auto backend = boost::make_shared<boost::log::sinks::text_ostream_backend>();
	backend->add_stream(boost::shared_ptr<std::ostream>(&out, boost::null_deleter()));

	// Create a sink and set its format
	if(lvl >= LogLevel::LEVEL_ONE)
	{
		auto error_sink =
		    boost::make_shared<boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend>>(backend);
		error_sink->set_filter(boost::log::trivial::severity == boost::log::trivial::severity_level::error);
		error_sink->set_formatter(
		    boost::log::expressions::stream
		    << C_B_WHITE << "[" << boost::log::expressions::attr<boost::posix_time::ptime>("TimeStamp") << "]"
		    << C_B_RED << " [ERROR] " << C_DEF << boost::log::expressions::smessage
		);
		n_LogSettings->add_sink(error_sink);
	}
	if(lvl >= LogLevel::LEVEL_TWO)
	{
		auto warning_sink =
		    boost::make_shared<boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend>>(backend);
		warning_sink->set_filter(boost::log::trivial::severity == boost::log::trivial::severity_level::warning);
		warning_sink->set_formatter(
		    boost::log::expressions::stream
		    << C_B_WHITE << "[" << boost::log::expressions::attr<boost::posix_time::ptime>("TimeStamp") << "]"
		    << C_B_YELLOW << " [WARNING] " << C_DEF << boost::log::expressions::smessage
		);
		n_LogSettings->add_sink(warning_sink);
	}
	if(lvl >= LogLevel::LEVEL_THREE)
	{
		auto info_sink =
		    boost::make_shared<boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend>>(backend);
		info_sink->set_filter(boost::log::trivial::severity == boost::log::trivial::severity_level::info);
		info_sink->set_formatter(
		    boost::log::expressions::stream
		    << C_B_WHITE << "[" << boost::log::expressions::attr<boost::posix_time::ptime>("TimeStamp") << "]"
		    << C_B_BLUE << " [INFO] " << C_DEF << boost::log::expressions::smessage
		);
		n_LogSettings->add_sink(info_sink);
	}
	if(lvl >= LogLevel::LEVEL_FOUR)
	{
		auto debug_sink =
		    boost::make_shared<boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend>>(backend);
		debug_sink->set_filter(boost::log::trivial::severity == boost::log::trivial::severity_level::debug);
		debug_sink->set_formatter(
		    boost::log::expressions::stream
		    << C_B_WHITE << "[" << boost::log::expressions::attr<boost::posix_time::ptime>("TimeStamp") << "]"
		    << C_B_GREEN << " [DEBUG] " << C_DEF << boost::log::expressions::smessage
		);
		n_LogSettings->add_sink(debug_sink);
	}
	boost::log::add_common_attributes();
	n_activeLevel = lvl;
}
void log::Reset()
{
	if(n_LogSettings)
	{
		n_LogSettings->remove_all_sinks();
		n_LogSettings.reset();  // sets n_LogSettings to nullptr
	}
	n_activeLevel = log::LogLevel::LEVEL_TWO;
}

void log::Debug(std::string const& format, ...)
{
	if(n_LogSettings && (n_activeLevel == LogLevel::LEVEL_FOUR))
	{
		va_list args1;
		// cppcheck-suppress va_start_referencePassed
		va_start(args1, format);
		va_list args2;
		va_copy(args2, args1);
		// Reason for suppression – Fixed CWE-134 as we are taking account of ‘\0’ (adding +1 to buffer length)
		// and make sure caller using the constant string for argument "format"
		// Flawfinder: ignore
		std::vector<char> buffer(1 + std::vsnprintf(nullptr, 0, format.c_str(), args1));
		va_end(args1);
		// Reason for suppression – Fixed CWE-134 as we are taking account of ‘\0’ (adding +1 to buffer length)
		// and make sure caller using the constant string for argument "format"
		// Flawfinder: ignore
		std::vsnprintf(buffer.data(), buffer.size(), format.c_str(), args2);
		va_end(args2);

		BOOST_LOG_TRIVIAL(debug) << buffer.data();
	}
}
void log::Info(std::string const& format, ...)
{
	if(n_LogSettings && (n_activeLevel >= LogLevel::LEVEL_THREE))
	{
		va_list args1;
		// cppcheck-suppress va_start_referencePassed
		va_start(args1, format);
		va_list args2;
		va_copy(args2, args1);
		// Reason for suppression – Fixed CWE-134 as we are taking account of ‘\0’ (adding +1 to buffer length)
		// and make sure caller using the constant string for argument "format"
		// Flawfinder: ignore
		std::vector<char> buffer(1 + std::vsnprintf(nullptr, 0, format.c_str(), args1));
		va_end(args1);
		// Reason for suppression – Fixed CWE-134 as we are taking account of ‘\0’ (adding +1 to buffer length)
		// and make sure caller using the constant string for argument "format"
		// Flawfinder: ignore
		std::vsnprintf(buffer.data(), buffer.size(), format.c_str(), args2);
		va_end(args2);

		BOOST_LOG_TRIVIAL(info) << buffer.data();
	}
}
void log::Warning(std::string const& format, ...)
{
	if(n_LogSettings && (n_activeLevel >= LogLevel::LEVEL_TWO))
	{
		va_list args1;
		// cppcheck-suppress va_start_referencePassed
		va_start(args1, format);
		va_list args2;
		va_copy(args2, args1);
		// Reason for suppression – Fixed CWE-134 as we are taking account of ‘\0’ (adding +1 to buffer length)
		// and make sure caller using the constant string for argument "format"
		// Flawfinder: ignore
		std::vector<char> buffer(1 + std::vsnprintf(nullptr, 0, format.c_str(), args1));
		va_end(args1);
		// Reason for suppression – Fixed CWE-134 as we are taking account of ‘\0’ (adding +1 to buffer length)
		// and make sure caller using the constant string for argument "format"
		// Flawfinder: ignore
		std::vsnprintf(buffer.data(), buffer.size(), format.c_str(), args2);
		va_end(args2);

		BOOST_LOG_TRIVIAL(warning) << buffer.data();
	}
}
void log::Error(std::string const& format, ...)
{
	va_list args1;
	// cppcheck-suppress va_start_referencePassed
	va_start(args1, format);
	va_list args2;
	va_copy(args2, args1);
	// Reason for suppression – Fixed CWE-134 as we are taking account of ‘\0’ (adding +1 to buffer length)
	// and make sure caller using the constant string for argument "format"
	// Flawfinder: ignore
	std::vector<char> buffer(1 + std::vsnprintf(nullptr, 0, format.c_str(), args1));
	va_end(args1);
	// Reason for suppression – Fixed CWE-134 as we are taking account of ‘\0’ (adding +1 to buffer length)
	// and make sure caller using the constant string for argument "format"
	// Flawfinder: ignore
	std::vsnprintf(buffer.data(), buffer.size(), format.c_str(), args2);
	va_end(args2);

	if(n_LogSettings && (n_activeLevel >= LogLevel::LEVEL_ONE))
	{
		BOOST_LOG_TRIVIAL(error) << buffer.data();
	}
	throw std::runtime_error(buffer.data());
}
}  // namespace middleware
