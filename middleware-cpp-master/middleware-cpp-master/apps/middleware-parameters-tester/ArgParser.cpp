// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ArgParser.cpp
// Description: Command-line argument parsing for middleware-parameters-tester.

#include "ArgParser.h"
#include <chrono>
#include <cstdio>
#include <stdexcept>
#include <string>

namespace
{
/// @brief Parses a delay string such as "100ms" or "5000" into milliseconds.
///        If no "ms" suffix is present the value is assumed to already be in milliseconds.
std::chrono::milliseconds ParseDelay(std::string const& s)
{
	std::string val = s;
	if(val.size() >= 2 && val.substr(val.size() - 2) == "ms")
	{
		val = val.substr(0, val.size() - 2);
	}
	return std::chrono::milliseconds(std::stoul(val));
}
}  // namespace

namespace mpt
{
void PrintUsage(char const* program)
{
	fprintf(
	    stderr,
	    "Usage:\n"
	    "  %s help\n"
	    "  %s pub  --name <name> [--double_value|--string_value|--binary_value|--boolean_value|--uinteger64_value <v>]"
	    " [--delay <Nms>]\n"
	    "  %s sub  --name <name> [--name <name> ...]\n"
	    "  %s read --name <name> [--name <name> ...]\n"
	    "  %s write --name <name>"
	    " [--double_value|--string_value|--binary_value|--boolean_value|--uinteger64_value <v>]\n",
	    program,
	    program,
	    program,
	    program,
	    program
	);
}

Args ParseArgs(int argc, char** argv)
{
	if(argc < 2)
	{
		PrintUsage(argv[0]);
		throw std::invalid_argument("Missing required action argument.");
	}

	Args args;
	args.action = argv[1];

	// Normalise help flags so main can dispatch uniformly.
	if(args.action == "--help" || args.action == "-h")
	{
		args.action = "help";
	}

	for(int i = 2; i < argc; ++i)
	{
		std::string const arg = argv[i];

		if(arg == "--name" && i + 1 < argc)
		{
			args.names.push_back(argv[++i]);
		}
		else if(arg == "--delay" && i + 1 < argc)
		{
			args.delay = ParseDelay(argv[++i]);
		}
		else if(arg == "--double_value" && i + 1 < argc)
		{
			args.has_double = true;
			args.double_val = std::stod(argv[++i]);
		}
		else if(arg == "--string_value" && i + 1 < argc)
		{
			args.has_string = true;
			args.string_val = argv[++i];
		}
		else if(arg == "--binary_value" && i + 1 < argc)
		{
			args.has_binary = true;
			args.binary_val = argv[++i];
		}
		else if(arg == "--boolean_value" && i + 1 < argc)
		{
			args.has_boolean     = true;
			std::string const bv = argv[++i];
			args.boolean_val     = (bv == "true" || bv == "1");
		}
		else if(arg == "--uinteger64_value" && i + 1 < argc)
		{
			args.has_uint64 = true;
			args.uint64_val = std::stoull(argv[++i]);
		}
	}

	if(args.names.empty() && args.action != "help")
	{
		throw std::invalid_argument("--name argument is required.");
	}

	if((args.action == "pub" || args.action == "write") && args.names.size() > 1)
	{
		throw std::invalid_argument("Only one --name is allowed for the '" + args.action + "' action.");
	}

	bool const has_value = args.has_double || args.has_string || args.has_binary || args.has_boolean || args.has_uint64;
	if((args.action == "pub" || args.action == "write") && !has_value)
	{
		throw std::invalid_argument(
		    "A value argument is required for '" + args.action +
		    "': --double_value, --string_value, --binary_value, --boolean_value, or --uinteger64_value."
		);
	}

	return args;
}
}  // namespace mpt
