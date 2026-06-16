// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ArgParser.h
// Description: Command-line argument parsing for middleware-parameters-tester.

#pragma once

#include "Args.h"

namespace mpt
{
/// @brief Prints usage information to stderr.
void PrintUsage(char const* program);

/// @brief Parses argc/argv into an Args struct.
/// @throws std::invalid_argument when required arguments are missing or the action is unknown.
Args ParseArgs(int argc, char** argv);
}  // namespace mpt
