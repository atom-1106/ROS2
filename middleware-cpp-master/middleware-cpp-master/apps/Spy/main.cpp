// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: main.cpp
// Description: Middleware Data Trace main

#include <condition_variable>
#include <iostream>
#include <mutex>
#include "SpyApp.h"

int main(int argc, char** argv)
{
	std::cerr << "Note: Publishers being removed may not (ever) be printed if they do not exit cleanly" << std::endl;
	std::cerr << "      This appears to include those that lazily teardown" << std::endl;
	SpyApp spy{};

	std::mutex mutex{};
	std::condition_variable condition{};
	std::unique_lock<std::mutex> lock{mutex};
	condition.wait(lock, [] { return false; });
	return 0;
}
