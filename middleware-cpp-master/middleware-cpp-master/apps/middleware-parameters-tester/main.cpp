// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: main.cpp
// Description: Entry point for middleware-parameters-tester.

#include <cstdio>
#include <exception>
#include <iostream>
#include <stdexcept>
#include "Actions.h"
#include "ArgParser.h"
#include "ParameterServiceFactory.h"

int main(int argc, char** argv)
{
	try
	{
		::middleware::parameter::Factory::SetLogLevel(std::cout, 4);

		auto const args = mpt::ParseArgs(argc, argv);

		if(args.action == "help")
		{
			mpt::PrintUsage(argv[0]);
		}
		else if(args.action == "pub")
		{
			mpt::RunPub(args);
		}
		else if(args.action == "sub")
		{
			mpt::RunSub(args);
		}
		else if(args.action == "read")
		{
			mpt::RunRead(args);
		}
		else if(args.action == "write")
		{
			mpt::RunWrite(args);
		}
		else
		{
			mpt::PrintUsage(argv[0]);
			throw std::invalid_argument(
			    "Unknown action: '" + args.action + "'. Expected: help, pub, sub, read, write."
			);
		}
	}
	catch(std::exception const& e)
	{
		fprintf(stderr, "Error: %s\n", e.what());
		return 1;
	}

	return 0;
}
