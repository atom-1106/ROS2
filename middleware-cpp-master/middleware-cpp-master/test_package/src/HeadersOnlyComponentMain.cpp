// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: HeaderOnlyMain.cpp
// Description: Test Package for cate_middleware component MiddlewareServiceHeaders

#include <IBasicParameter.h>
#include <IDiagnosticService.h>
#include <memory>

namespace
{

static constexpr char n_SomeIdentity[] = "Jeff";
class TestBasicParameter : public ::middleware::parameter::IBasic
{
public:
	virtual ~TestBasicParameter() override = default;
	::middleware::parameter::Identity Identity() override
	{
		return n_SomeIdentity;
	}
	::middleware::parameter::Unit Unit() override
	{
		return ::middleware::parameter::Unit{};
	}
};
class TestDiagnosticService : public ::middleware::diagnostics::IService
{
public:
	// outputs
	using IClient = ::middleware::diagnostics::ICodeClient;
	using IServer = ::middleware::diagnostics::ICodeServer;
	// types
	using ClientConfiguration = ::middleware::diagnostics::CodeConsumer;
	using Codes               = ::cat::middleware::diagnostics::Codes;

	virtual ~TestDiagnosticService() override = default;
	std::unique_ptr<IClient> CreateCodeClient(ClientConfiguration&& client) override
	{
		return nullptr;
	}
	std::unique_ptr<IServer>
	CreateCodeServer(Codes const& current, ::middleware::diagnostics::ICodeServer::OnRequest&& on_request) override
	{
		return nullptr;
	}
};
}  // namespace

int main()
{
	// Parameters
	std::cout << "[CHECK] Parameter Service Headers...\n";
	TestBasicParameter parameter{};
	if(parameter.Identity() != std::string(n_SomeIdentity))
	{
		std::cerr << "[FAILED] Parameter Service Headers\n";
		return 1;
	}
	std::cout << "[PASS] Parameter Service Headers\n";

	// Diagnostics
	std::cout << "[CHECK] Diagnostics Service Headers...\n";
	TestDiagnosticService diagnostic{};
	::middleware::diagnostics::CodeConsumer config{};
	if(diagnostic.CreateCodeClient(std::move(config)))
	{
		std::cerr << "[FAILED] Diagnostics Service Headers\n";
		return 1;
	}
	std::cout << "[PASS] Diagnostics Service Headers\n";

	return 0;
}
