// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: LogUtility.h
// Description: Middleware Data Trace Callbacks

#ifndef LogUtility_H
#define LogUtility_H

#include <middleware_diagnostics.pb.h>
#include <middleware_parameter.pb.h>

#include <iomanip>
#include <iostream>
#include <sstream>

namespace utility
{
static void LogParameterData(std::string const& topic, ::cat::middleware::parameter::Data const& data)
{
	std::stringstream ss{};
	ss << "Parameter Data Published:" << std::endl;
	ss << "\tParameter: " << topic << std::endl;
	// TODO: after Middleware redesign or update to 3.X, give publisher GUID here

	switch(data.value().selection_case())
	{
		case ::cat::middleware::parameter::Value::kDoubleValue:
		{
			ss << "\tType:      Double\n";
			ss << "\tValue:     " << data.value().double_value() << std::endl;
			break;
		}
		case ::cat::middleware::parameter::Value::kUinteger64Value:
		{
			ss << "\tType:      Uint64\n";
			ss << "\tValue:     " << data.value().uinteger64_value() << std::endl;
			break;
		}
		case ::cat::middleware::parameter::Value::kStringValue:
		{
			ss << "\tType:      String\n";
			ss << "\tValue:     " << data.value().string_value() << std::endl;
			break;
		}
		case ::cat::middleware::parameter::Value::kBooleanValue:
		{
			ss << "\tType:      Boolean\n";
			ss << "\tValue:     " << ((data.value().boolean_value()) ? "True" : "False") << std::endl;
			break;
		}
		case ::cat::middleware::parameter::Value::kBinaryValue:
		{
			ss << "\tType:      Binary\n";
			ss << "\tValue:     ";
			std::vector<uint8_t> binary{
			    std::cbegin(data.value().binary_value()), std::cend(data.value().binary_value())
			};
			for(auto const& byte : binary)
			{
				ss << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << static_cast<int>(byte)
				   << std::dec;
			}
			ss << std::endl;
			break;
		}
		default:
		{
			ss << "\tType:      UNKNOWN\n";
			ss << "\tValue:     ERROR";
			break;
		}
	}
	std::cout << ss.str() << std::endl;
}
static std::string GetNetworkAddress(::cat::middleware::diagnostics::Code const& code)
{
	switch(code.network_address().network_address_case())
	{
		case ::cat::middleware::diagnostics::NetworkAddress::kSelf:
			return std::to_string(code.network_address().self().j1939().address());
		case ::cat::middleware::diagnostics::NetworkAddress::kJ1939Address:
			return std::to_string(code.network_address().j1939_address().address());
		default: return "UNKNOWN";
	}
}
static std::string GetCodeStatus(::cat::middleware::diagnostics::CodeStatus const& status)
{
	switch(status)
	{
		case ::cat::middleware::diagnostics::CODE_STATUS_NONE: return "NONE";
		case ::cat::middleware::diagnostics::CODE_STATUS_ACTIVE_ONLY: return "ACTIVE_ONLY";
		case ::cat::middleware::diagnostics::CODE_STATUS_ACTIVE_AND_LOGGED: return "ACTIVE_AND_LOGGED";
		case ::cat::middleware::diagnostics::CODE_STATUS_LOGGED_ONLY: return "LOGGED_ONLY";
		case ::cat::middleware::diagnostics::CODE_STATUS_PREVIOUSLY_ACTIVE: return "PREVIOUSLY_ACTIVE";
		default: return "UNKNOWN";
	}
}
static std::string GetInformation(::cat::middleware::diagnostics::Code const& code)
{
	std::stringstream log{};
	switch(code.information().information_case())
	{
		case ::cat::middleware::diagnostics::Information::kDiagnostic:
		{
			log << "\tCODE:    " << code.information().diagnostic().component_identifier() << std::endl;
			log << "\tWCI:     " << code.information().diagnostic().warning_category_indicator() << std::endl;
			log << "\tFMI:     " << code.information().diagnostic().failure_mode_identifier() << std::endl;
			log << "\tAR:      " << code.information().diagnostic().annunciation_requirement() << std::endl;
			log << "\tSTATUS:  " << GetCodeStatus(code.information().diagnostic().status()) << std::endl;
			break;
		}
		case ::cat::middleware::diagnostics::Information::kEvent:
		{
			log << "\tCODE:    " << code.information().event().event_identifier() << std::endl;
			log << "\tWCI:     " << code.information().event().warning_category_indicator() << std::endl;
			log << "\tAR:      " << code.information().event().annunciation_requirement() << std::endl;
			log << "\tSTATUS:  " << GetCodeStatus(code.information().event().status()) << std::endl;
			break;
		}
		case ::cat::middleware::diagnostics::Information::kPublicDtc:
		{
			log << "\tSPN:     " << code.information().public_dtc().suspect_parameter_number() << std::endl;
			log << "\tWCI:     " << code.information().public_dtc().warning_category_indicator() << std::endl;
			log << "\tFMI:     " << code.information().public_dtc().failure_mode_identifier() << std::endl;
			log << "\tAR:      " << code.information().public_dtc().annunciation_requirement() << std::endl;
			log << "\tOCC:     " << code.information().public_dtc().occurrence_count() << std::endl;
			log << "\tSTATUS:  " << GetCodeStatus(code.information().public_dtc().status()) << std::endl;
			break;
		}

		default: break;
	}
	return log.str();
}
static void LogDiagnosticCode(std::string const& topic, ::cat::middleware::diagnostics::Codes const& codes)
{
	std::stringstream ss{};
	ss << "Diagnostic Codes Published:" << std::endl;

	for(auto const& code : codes.codes())
	{
		ss << "\tKey:     " << code.code_key() << std::endl;
		ss << "\tAddress: " << GetNetworkAddress(code) << std::endl;
		ss << GetInformation(code) << "\n\n";
	}
	std::cout << ss.str() << std::endl;
}
}  // namespace utility

#endif  // LogUtility_H
