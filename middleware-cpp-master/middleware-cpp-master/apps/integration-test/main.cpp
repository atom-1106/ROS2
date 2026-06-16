// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: main.cpp
// Description: The test main for integration tests.

#include <gmock/gmock.h>

int main(int argc, char* argv[])
{
	::testing::InitGoogleMock(&argc, argv);

	return RUN_ALL_TESTS();
}
