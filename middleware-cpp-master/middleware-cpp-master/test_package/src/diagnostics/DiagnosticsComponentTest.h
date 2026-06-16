// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: DiagnosticsComponentTest.h
// Description: Test code for cate_middleware component MiddlewareDiagnosticsService

#ifndef DiagnosticsComponentTest_H
#define DiagnosticsComponentTest_H

#include <CodesDefines.h>
#include <CodesFactory.h>
#include <CodesStructures.h>
#include <ICodeClient.h>
#include <ICodeServer.h>
#include <IDiagnosticService.h>
#include <middleware_diagnostics.pb.h>

namespace test_package
{
class DiagnosticsComponentTest
{

public:
	DiagnosticsComponentTest() : m_spCodesService{::middleware::diagnostics::Factory::CreateDiagnosticService()}
	{
	}
	virtual ~DiagnosticsComponentTest() = default;
	bool Run()
	{
		std::cout << "[CHECK] Diagnostics Service...\n";
		::cat::middleware::diagnostics::Codes current{};
		auto spCodeServer = m_spCodesService->CreateCodeServer(
		    current,
		    [](auto, auto const&)
		    {
			    ::cat::middleware::diagnostics::AdditionalInformation info{};
			    return info;
		    }
		);
		if(spCodeServer)
		{
			std::cout << "[PASS] Diagnostics Service\n";
			return true;
		}
		else
		{
			std::cerr << "[FAILED] Diagnostics Service\n";
			return false;
		}
	}

private:
	std::unique_ptr<::middleware::diagnostics::IService> m_spCodesService;
};
}  // namespace test_package

#endif  // DiagnosticsComponentTest_H
