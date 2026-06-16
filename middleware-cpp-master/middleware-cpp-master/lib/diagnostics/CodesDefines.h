// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: CodesDefines.h
// Description: Defines for the Diagnostics Service.

#ifndef CodesDefines_H
#define CodesDefines_H

#include <chrono>
#include <cstdint>
#include <string>

namespace middleware
{
namespace diagnostics
{
static constexpr char connection_name[]                       = "Diagnostics";
static constexpr char additionalinformation_connection_name[] = "Diagnostics AdditionalInformation";
static constexpr uint32_t domainIdForReadWrite{1};
static constexpr uint32_t threadPoolSizeForReadWrite{5};
static std::chrono::seconds const clientRequestTimeout{10};
}  // namespace diagnostics
}  // namespace middleware
#endif  // CodesDefines_H
