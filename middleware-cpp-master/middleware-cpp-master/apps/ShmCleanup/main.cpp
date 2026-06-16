// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: main.cpp
// Description: Cleans up shared memory, used for testing.

#include "ShmCleanup.h"

int main(int argc, char** argv)
{
	// an argument is passed when this is called from the unit tests
	// this cleans up a defined directory (instead of /dev/shm) for everything but semaphores
	::middleware::ShmCleanup((argc >= 2) ? std::string{argv[1]} : "/dev/shm");
	return 0;
}
