// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: main.cpp
// Description: Test Package for cate_middleware component MiddlewareParameterService

#include "ParametersComponentTest.h"

int main()
{
	::test_package::ParametersComponentTest tester{"Components", "Hello"};
	return (tester.Run() ? 0 : 1);
}
