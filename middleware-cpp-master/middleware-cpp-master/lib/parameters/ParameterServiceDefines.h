// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ParameterServiceDefines.h
// Description: {Blitz} Defined Structures for the middleware.

#ifndef ParameterServiceDefines_H
#define ParameterServiceDefines_H

#include <cstdint>
#include <string>
#include "ParameterServiceUnits.h"

namespace cat
{
namespace middleware
{
namespace parameter
{
class Data;
class Info;
class Parameter;
}  // namespace parameter
}  // namespace middleware
}  // namespace cat

namespace middleware
{
namespace parameter
{
using Data      = ::cat::middleware::parameter::Data;
using Info      = ::cat::middleware::parameter::Info;
using Parameter = ::cat::middleware::parameter::Parameter;
using Identity  = std::string;

enum Status
{
	UNSPECIFIED,
	SUCCESS,
	FAILURE
};
struct Metadata
{
	::middleware::parameter::Unit unit{::middleware::parameter::Unit::none};
};
}  // namespace parameter
}  // namespace middleware

#endif  // ParameterServiceDefines_H
