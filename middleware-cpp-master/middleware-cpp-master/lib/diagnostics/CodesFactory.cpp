// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: CodesFactory.cpp
// Description: Interface API for external users.

#include "CodesFactory.h"
#include "DiagnosticService.h"

#include <memory>

namespace
{
}  // namespace

namespace middleware
{
namespace diagnostics
{
std::unique_ptr<::middleware::diagnostics::IService> Factory::CreateDiagnosticService()
{
	return std::make_unique<::middleware::diagnostics::Service>();
}

}  // namespace diagnostics
}  // namespace middleware
