// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: PackageMain.cpp
// Description: Test Package for cate_middleware

#include "DiagnosticsComponentTest.h"
#include "ParametersComponentTest.h"

int main()
{
	::test_package::ParametersComponentTest parameters{"Primary", "Goodbye"};
	if(!parameters.Run())
	{
		return 1;
	}
	::test_package::DiagnosticsComponentTest diagnostics{};
	if(!diagnostics.Run())
	{
		return 1;
	}
	return 0;
}
