// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ParameterEndpointConfigurationUtilities.cpp
// Description: Temporary construct of the APIs static connections configuration

#include "ParameterEndpointConfigurationUtilities.h"
#include "ParameterServiceStructures.h"
#include "SystemUtilities.h"

#include <algorithm>
#include <iostream>
#include <pugixml.hpp>
#include <sstream>

namespace
{
static constexpr char n_DefaultServerSourceName[] = "DataLinkEngine";
static constexpr char n_UserTestConfiguration[]   = "/opt/appdata/Middleware/PARAMETER_SERVICE_TEST_MODE.xml";
static std::vector<::middleware::parameter::Information> const n_DefaultDataEngineSource{
    {"EngineSpeed", "DataLinkEngine", ::middleware::parameter::DataType::DATA_TYPE_DOUBLE},
    {"BucketHeight", "DataLinkEngine", ::middleware::parameter::DataType::DATA_TYPE_DOUBLE},
    {"SpeedLimit", "DataLinkEngine", ::middleware::parameter::DataType::DATA_TYPE_DOUBLE},
    {"AirFilter1Restriction", "DataLinkEngine", ::middleware::parameter::DataType::DATA_TYPE_DOUBLE},
    {"AirFilter2Restriction", "DataLinkEngine", ::middleware::parameter::DataType::DATA_TYPE_DOUBLE},
    {"BoostPressureGauge", "DataLinkEngine", ::middleware::parameter::DataType::DATA_TYPE_DOUBLE},
    {"ServiceMeterHours", "DataLinkEngine", ::middleware::parameter::DataType::DATA_TYPE_DOUBLE},
    {"ServiceBrakePedalPositionStatus", "DataLinkEngine", ::middleware::parameter::DataType::DATA_TYPE_UINT64},
    {"DelayedEngineShutdownStatus", "DataLinkEngine", ::middleware::parameter::DataType::DATA_TYPE_UINT64},
    {"EngineIdleShutdownInstallationStatus", "DataLinkEngine", ::middleware::parameter::DataType::DATA_TYPE_BOOL},
    {"MachineSerialNumber", "DataLinkEngine", ::middleware::parameter::DataType::DATA_TYPE_STRING},
    {"SoftwarePartNumber", "DataLinkEngine", ::middleware::parameter::DataType::DATA_TYPE_STRING},
    {"ProductID", "DataLinkEngine", ::middleware::parameter::DataType::DATA_TYPE_STRING},
};
static std::vector<::middleware::parameter::Information> const n_DefaultCoreTelematicsSource{
    {"GPSData", "CoreTelematics", ::middleware::parameter::DataType::DATA_TYPE_BINARY},
    {"RadioStatus", "CoreTelematics", ::middleware::parameter::DataType::DATA_TYPE_BINARY},
    {"ECMSerialNumber", "CoreTelematics", ::middleware::parameter::DataType::DATA_TYPE_STRING},
    {"HardwarePartNumber", "CoreTelematics", ::middleware::parameter::DataType::DATA_TYPE_STRING},
};
static std::vector<::middleware::parameter::Information> const n_DefaultDisplaySource{
    {"Brightness", "Display", ::middleware::parameter::DataType::DATA_TYPE_UINT64},
    {"BatteryLevel", "Display", ::middleware::parameter::DataType::DATA_TYPE_DOUBLE},
    {"Language", "Display", ::middleware::parameter::DataType::DATA_TYPE_UINT64},
};
static std::vector<::middleware::parameter::Information> const n_DefaultTMSASource{
    {"EquipmentOperatingStatus", "TMSA", ::middleware::parameter::DataType::DATA_TYPE_BOOL},
    {"LifetimeOperatingHours", "TMSA", ::middleware::parameter::DataType::DATA_TYPE_DOUBLE},
    {"ActiveConfigurationPackages", "TMSA", ::middleware::parameter::DataType::DATA_TYPE_BINARY},
};
static std::vector<::middleware::parameter::Information> const n_DefaultBlueKeySource{
    {"UserProfileName", "SomeBlueKey", ::middleware::parameter::DataType::DATA_TYPE_STRING},
    {"ProfileReadyStatus", "SomeBlueKey", ::middleware::parameter::DataType::DATA_TYPE_BINARY},
    {"ActiveProfileCount", "SomeBlueKey", ::middleware::parameter::DataType::DATA_TYPE_UINT64},
};
static ::middleware::parameter::SourceTableConnections const n_DefaultParameterSourceInformationDirectory{
    // Data Link
    {"DataLinkEngine", n_DefaultDataEngineSource},
    // CoreTelematics
    {"CoreTelematics", n_DefaultCoreTelematicsSource},
    // Display
    {"Display", n_DefaultDisplaySource},
    // TMSA
    {"TMSA", n_DefaultTMSASource},
    // Blue Key
    {"SomeBlueKey", n_DefaultBlueKeySource},
};
}  // namespace

namespace middleware
{
namespace parameter
{
std::map<DataType, std::string> const n_ParameterDataTypeAsString{
    {DataType::DATA_TYPE_UNSPECIFIED, "unspecified"},
    {DataType::DATA_TYPE_DOUBLE, "double"},
    {DataType::DATA_TYPE_STRING, "string"},
    {DataType::DATA_TYPE_BINARY, "binary"},
    {DataType::DATA_TYPE_BOOL, "bool"},
    {DataType::DATA_TYPE_UINT64, "unsigned"},
    {DataType::DATA_TYPE_MAX, "max"},
};
std::map<std::string, DataType> const n_ParameterDataTypeAsEnum{
    {"unspecified", DataType::DATA_TYPE_UNSPECIFIED},
    {"double", DataType::DATA_TYPE_DOUBLE},
    {"string", DataType::DATA_TYPE_STRING},
    {"binary", DataType::DATA_TYPE_BINARY},
    {"bool", DataType::DATA_TYPE_BOOL},
    {"unsigned", DataType::DATA_TYPE_UINT64},
    {"max", DataType::DATA_TYPE_MAX},
};
bool Domain::Load(::middleware::parameter::ParameterService const& configuration)
{
	LoadConnections();
	return ValidateConnections(configuration);
}
bool Domain::ValidateConnections(::middleware::parameter::ParameterService const& configuration)
{
	bool result = true;
	// Gather other connections
	std::vector<std::string> other_connections{};
	other_connections.reserve(
	    configuration.subscribed_parameters.size() + configuration.client_write_parameters.size() +
	    configuration.client_read_parameters.size()
	);

	GetAllParameterIds(configuration.subscribed_parameters, other_connections);
	GetAllParameterIds(configuration.client_write_parameters, other_connections);
	GetAllParameterIds(configuration.client_read_parameters, other_connections);
	// Remove duplicates
	std::sort(other_connections.begin(), other_connections.end());
	auto other_dups = std::unique(other_connections.begin(), other_connections.end());
	other_connections.erase(other_dups, other_connections.end());

	// Find Self in list of owners
	auto selfFound = n_ParameterSourceInformationDirectory.find(configuration.server_identity);
	if(selfFound == std::cend(n_ParameterSourceInformationDirectory))
	{
		::middleware::log::Debug(
		    "[ValidateConnections] User identity [%s] was not found in connections table.",
		    configuration.server_identity.c_str()
		);
		result = false;
	}
	else
	{
		// Gather owned connections
		std::vector<std::string> owned_connections{};
		owned_connections.reserve(
		    configuration.published_parameters.size() + configuration.server_write_parameters.size() +
		    configuration.server_read_parameters.size()
		);
		GetAllParameterIds(configuration.published_parameters, owned_connections);
		GetAllParameterIds(configuration.server_write_parameters, owned_connections);
		GetAllParameterIds(configuration.server_read_parameters, owned_connections);
		// Remove duplicates
		std::sort(owned_connections.begin(), owned_connections.end());
		auto owned_dups = std::unique(owned_connections.begin(), owned_connections.end());
		owned_connections.erase(owned_dups, owned_connections.end());
		// Verify owned connection names
		for(auto const& connection : owned_connections)
		{
			auto paramFound = std::find_if(
			    std::cbegin(selfFound->second),
			    std::cend(selfFound->second),
			    [name = connection](auto const& param) { return param.parameter_name == name; }
			);
			if(paramFound == std::cend(selfFound->second))
			{
				::middleware::log::Debug(
				    "[ValidateConnections] User identity [%s] does not own [%s] in the connections table.",
				    configuration.server_identity.c_str(),
				    connection.c_str()
				);
				result = false;
			}
		}
	}
	// Verify other connection names
	for(auto const& connection : other_connections)
	{
		bool was_found{false};
		for(auto const& entry : n_ParameterSourceInformationDirectory)
		{
			auto paramFound = std::find_if(
			    std::cbegin(entry.second),
			    std::cend(entry.second),
			    [name = connection](auto const& param) { return param.parameter_name == name; }
			);
			if(paramFound != std::cend(entry.second))
			{
				was_found = true;
				break;
			}
		}
		if(!was_found)
		{
			::middleware::log::Debug(
			    "[ValidateConnections] Parameter connection [%s] was not found in connections table.",
			    connection.c_str()
			);
			result = false;
		}
	}
	// Log connections table and return result
	if(!result)
	{
		LogLoadedConnections();
	}
	return result;
}

void Domain::LoadConnections()
{
	if(n_ParameterSourceInformationDirectory.empty())
	{
		pugi::xml_document document;
		if(document.load_file(n_UserTestConfiguration))
		{
			::middleware::log::Debug(
			    "[LoadConnections] table loaded from PARAMETER_SERVICE_TEST_MODE for Client Read & Write"
			);
			auto domainsNode = document.select_node("/domains").node();
			if(domainsNode.empty())
			{
				::middleware::log::Debug("[LoadConnections] No domains found in configuration file.");
			}
			else
			{
				for(auto const& domain : domainsNode.children())
				{
					auto owner_name                                   = domain.attribute("name").as_string();
					n_ParameterSourceInformationDirectory[owner_name] = std::vector<Information>{};
					for(auto const& parameter : domain.children())
					{
						std::string parameter_name = parameter.attribute("name").as_string();
						std::string value_type     = parameter.attribute("value_type").as_string();
						auto value_enum            = ::middleware::parameter::DATA_TYPE_UNSPECIFIED;

						auto found = n_ParameterDataTypeAsEnum.find(value_type);
						if(found != std::cend(n_ParameterDataTypeAsEnum))
						{
							value_enum = found->second;
						};
						n_ParameterSourceInformationDirectory[owner_name].emplace_back(
						    parameter_name, owner_name, value_enum
						);
					}
				}
			}
		}
	}
	if(n_ParameterSourceInformationDirectory.empty())
	{
		::middleware::log::Debug("[LoadConnections] Table loaded from defaults for Client Read & Write");
		n_ParameterSourceInformationDirectory = n_DefaultParameterSourceInformationDirectory;
	}
}
std::string Domain::GetSubscriptionByParameterName(std::string const& name)
{
	return ("ParameterService::" + FormatName(name));
}
std::string Domain::FormatName(std::string const& name)
{
	std::string formatted_name{};
	std::copy_if(
	    std::cbegin(name),
	    std::cend(name),
	    std::back_inserter(formatted_name),
	    [](unsigned char x) { return !std::isspace(x); }
	);
	return formatted_name;
}
SourceTableConnections Domain::GetClientConnections(::middleware::parameter::ParameterService const& configuration)
{
	SourceTableConnections clientConnections{};
	std::vector<std::string> configuredConnections{};
	GetAllParameterIds(configuration.client_write_parameters, configuredConnections);
	GetAllParameterIds(configuration.client_read_parameters, configuredConnections);

	if(configuredConnections.empty())
	{
		clientConnections[n_DefaultServerSourceName];
		return clientConnections;
	}

	for(auto const& source : n_ParameterSourceInformationDirectory)
	{
		if(source.first != configuration.server_identity)
		{
			for(auto const& info : source.second)
			{
				auto found = std::find(
				    std::cbegin(configuredConnections), std::cend(configuredConnections), info.parameter_name
				);
				if(found != std::cend(configuredConnections))
				{
					clientConnections[source.first].emplace_back(info);
				}
			}
		}
	}

	for(auto const& connection : configuredConnections)
	{
		bool const foundInSource = std::any_of(
		    std::cbegin(clientConnections),
		    std::cend(clientConnections),
		    [&connection](auto const& entry)
		    {
			    return std::any_of(
			        std::cbegin(entry.second),
			        std::cend(entry.second),
			        [&connection](auto const& info) { return info.parameter_name == connection; }
			    );
		    }
		);
		if(!foundInSource)
		{
			::middleware::log::Warning(
			    "[GetClientConnections] Parameter [%s] was not found in any source. Defaulting to DataLinkEngine.",
			    connection.c_str()
			);
			clientConnections[n_DefaultServerSourceName].emplace_back(
			    Information{connection, n_DefaultServerSourceName, DataType::DATA_TYPE_UNSPECIFIED}
			);
		}
	}

	return clientConnections;
}
// Note: Not used but this file is primarily a placeholder for real implementation
// cppcheck-suppress unusedFunction
void Domain::LogLoadedConnections()
{
	std::stringstream ss{};
	for(auto const& owner : n_ParameterSourceInformationDirectory)
	{
		ss << "[Owner] name=\"" << owner.first << "\"\n";
		for(auto const& parameter : owner.second)
		{
			ss << "\t[Parameter] name=\"" << parameter.parameter_name << "\" type=\""
			   << n_ParameterDataTypeAsString.at(parameter.value_type) << "\"\n";
		}
	}
	::middleware::log::Debug("%s", ss.str().c_str());
}
template <typename T>
void Domain::GetAllParameterIds(std::vector<T> const& parameters, std::vector<std::string>& gathered)
{
	std::transform(
	    std::cbegin(parameters),
	    std::cend(parameters),
	    std::back_inserter(gathered),
	    [](auto const& entry) { return FormatName(entry.parameter_id); }
	);
}
}  // namespace parameter
}  // namespace middleware
