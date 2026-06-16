// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: main.cpp
// Description: Test Package for cate_middleware component MiddlewareDiagnosticsService

#include "DiagnosticsComponentTest.h"

int main()
{
	::test_package::DiagnosticsComponentTest tester{};
	return (tester.Run() ? 0 : 1);
}
