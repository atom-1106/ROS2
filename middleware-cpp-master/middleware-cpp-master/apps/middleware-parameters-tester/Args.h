// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: Args.h
// Description: Command-line argument structure for middleware-parameters-tester.

#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace mpt
{
inline constexpr char k_ServerIdentity[] = "MiddlewareParametersTester";

/// @brief Holds the parsed command-line arguments.
struct Args
{
	std::string action{};
	std::vector<std::string> names{};
	std::chrono::milliseconds delay{5000};
	bool has_double{false};
	double double_val{0.0};
	bool has_string{false};
	std::string string_val{};
	bool has_binary{false};
	std::string binary_val{};
	bool has_boolean{false};
	bool boolean_val{false};
	bool has_uint64{false};
	uint64_t uint64_val{0};
};
}  // namespace mpt
