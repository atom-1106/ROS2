// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareTests.cpp
// Description: The test main for tests related to the Parameter Service API.

#include <gmock/gmock.h>

int main(int argc, char* argv[])
{
	// Clear generated Fast DDS content
	std::ignore = std::system("rm -f /dev/shm/*fast*");

#ifdef NDEBUG
	testing::GTEST_FLAG(filter) = "-*DeathTest*:";
#endif
	::testing::InitGoogleMock(&argc, argv);

	auto result = RUN_ALL_TESTS();

	// Clear generated Fast DDS content
	std::ignore = std::system("rm -f /dev/shm/*fast*");
	return result;
}
