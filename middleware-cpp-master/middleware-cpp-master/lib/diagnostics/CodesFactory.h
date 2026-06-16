// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: CodesFactory.h
// Description: Interface API for external users.

#ifndef CodesFactory_H
#define CodesFactory_H

#include "IDiagnosticService.h"

namespace middleware
{
namespace diagnostics
{
class Factory
{
public:
	// Any number of receivers can be created.
	static std::unique_ptr<::middleware::diagnostics::IService> CreateDiagnosticService();
};
}  // namespace diagnostics
}  // namespace middleware
#endif  // CodesFactory_H
