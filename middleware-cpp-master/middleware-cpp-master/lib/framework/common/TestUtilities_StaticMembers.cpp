// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: TestUtilities_StaticMembers.cpp
// Description: Contains wrappers for testability.

#include "TestUtilities.h"

namespace middleware
{
namespace testable
{

std::unique_ptr<ChronoImpl> ChronoImpl::m_spChronoInstance;

}  // namespace testable
}  // namespace middleware
