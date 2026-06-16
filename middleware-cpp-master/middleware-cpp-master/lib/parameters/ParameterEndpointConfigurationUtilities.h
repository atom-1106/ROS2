// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ParameterEndpointConfigurationUtilities.h
// Description: Temporary construct of the APIs static connections configuration

#ifndef ParameterEndpointConfigurationUtilities_H
#define ParameterEndpointConfigurationUtilities_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace middleware
{
namespace parameter
{
enum DataType
{
	DATA_TYPE_UNSPECIFIED = 0,
	DATA_TYPE_DOUBLE,
	DATA_TYPE_STRING,
	DATA_TYPE_BINARY,
	DATA_TYPE_BOOL,
	DATA_TYPE_UINT64,
	DATA_TYPE_MAX  // Always last
};

struct Information
{
	std::string parameter_name{};
	std::string source_name{};
	DataType value_type{DataType::DATA_TYPE_UNSPECIFIED};
};
using SourceTableConnections = std::map<std::string, std::vector<Information>>;
extern std::map<DataType, std::string> const n_ParameterDataTypeAsString;
extern std::map<std::string, DataType> const n_ParameterDataTypeAsEnum;
inline SourceTableConnections n_ParameterSourceInformationDirectory{};

class ParameterService;

class Domain
{
public:
	static bool Load(::middleware::parameter::ParameterService const& configuration);
	static std::string GetSubscriptionByParameterName(std::string const& name);
	static SourceTableConnections GetClientConnections(::middleware::parameter::ParameterService const& configuration);
	static std::string FormatName(std::string const& name);

private:
	Domain() = delete;
	static void LoadConnections();
	static bool ValidateConnections(::middleware::parameter::ParameterService const& configuration);
	static void LogLoadedConnections();
	template <typename T>
	static void GetAllParameterIds(std::vector<T> const& parameters, std::vector<std::string>& gathered);
};
}  // namespace parameter
}  // namespace middleware
#endif  // ParameterEndpointConfigurationUtilities_H
