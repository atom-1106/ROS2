// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ParameterEndpointConfigurationUtilitiesTest.cpp
// Description: Unit Tests for ParameterEndpointConfigurationUtilities class

#include "ParameterEndpointConfigurationUtilities.cpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;
namespace
{

::middleware::parameter::ParameterService MakeValidDataLinkEngineConfig()
{
	::middleware::parameter::ParameterService config{};
	config.server_identity = "DataLinkEngine";
	config.published_parameters.push_back({"EngineSpeed"});
	config.published_parameters.push_back({"ServiceMeterHours"});
	return config;
}

::middleware::parameter::ParameterService MakeValidCoreTelematicsConfig()
{
	::middleware::parameter::ParameterService config{};
	config.server_identity = "CoreTelematics";
	config.published_parameters.push_back({"GPSData"});
	config.client_read_parameters.push_back({"EngineSpeed"});
	return config;
}

}  // namespace

class ParameterEndpointConfigurationUtilitiesTest : public ::testing::Test
{
protected:
	ParameterEndpointConfigurationUtilitiesTest()
	{
		// Reset the global directory before each test
		::middleware::parameter::n_ParameterSourceInformationDirectory.clear();
	}
	virtual ~ParameterEndpointConfigurationUtilitiesTest() = default;
};

// ---- FormatName ----

TEST_F(ParameterEndpointConfigurationUtilitiesTest, FormatNameNoWhitespaceReturnsSameString)
{
	auto result = ::middleware::parameter::Domain::FormatName("EngineSpeed");
	EXPECT_THAT(result, Eq("EngineSpeed"));
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, FormatNameWithSpacesRemovesSpaces)
{
	auto result = ::middleware::parameter::Domain::FormatName("Engine Speed");
	EXPECT_THAT(result, Eq("EngineSpeed"));
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, FormatNameWithLeadingAndTrailingSpacesRemovesSpaces)
{
	auto result = ::middleware::parameter::Domain::FormatName("  EngineSpeed  ");
	EXPECT_THAT(result, Eq("EngineSpeed"));
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, FormatNameWithTabsRemovesTabs)
{
	auto result = ::middleware::parameter::Domain::FormatName("Engine\tSpeed");
	EXPECT_THAT(result, Eq("EngineSpeed"));
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, FormatNameEmptyStringReturnsEmpty)
{
	auto result = ::middleware::parameter::Domain::FormatName("");
	EXPECT_THAT(result, Eq(""));
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, FormatNameOnlyWhitespaceReturnsEmpty)
{
	auto result = ::middleware::parameter::Domain::FormatName("   ");
	EXPECT_THAT(result, Eq(""));
}

// ---- GetSubscriptionByParameterName ----

TEST_F(ParameterEndpointConfigurationUtilitiesTest, GetSubscriptionByParameterNameValidNameReturnsPrefixedName)
{
	auto result = ::middleware::parameter::Domain::GetSubscriptionByParameterName("EngineSpeed");
	EXPECT_THAT(result, Eq("ParameterService::EngineSpeed"));
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, GetSubscriptionByParameterNameNameWithSpacesRemovesSpaces)
{
	auto result = ::middleware::parameter::Domain::GetSubscriptionByParameterName("Engine Speed");
	EXPECT_THAT(result, Eq("ParameterService::EngineSpeed"));
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, GetSubscriptionByParameterNameEmptyNameReturnsOnlyPrefix)
{
	auto result = ::middleware::parameter::Domain::GetSubscriptionByParameterName("");
	EXPECT_THAT(result, Eq("ParameterService::"));
}

// ---- Load / ValidateConnections ----

TEST_F(ParameterEndpointConfigurationUtilitiesTest, LoadValidServerIdentityWithOwnedParamsReturnsTrue)
{
	auto config = MakeValidDataLinkEngineConfig();
	bool result = ::middleware::parameter::Domain::Load(config);
	EXPECT_TRUE(result);
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, LoadUnknownServerIdentityReturnsFalse)
{
	::middleware::parameter::ParameterService config{};
	config.server_identity = "UnknownSource";
	bool result            = ::middleware::parameter::Domain::Load(config);
	EXPECT_FALSE(result);
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, LoadEmptyServerIdentityReturnsFalse)
{
	::middleware::parameter::ParameterService config{};
	config.server_identity = "";
	bool result            = ::middleware::parameter::Domain::Load(config);
	EXPECT_FALSE(result);
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, LoadValidServerIdentityWithUnownedPublishedParamReturnsFalse)
{
	::middleware::parameter::ParameterService config{};
	config.server_identity = "DataLinkEngine";
	config.published_parameters.push_back({"GPSData"});  // GPSData belongs to CoreTelematics
	bool result = ::middleware::parameter::Domain::Load(config);
	EXPECT_FALSE(result);
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, LoadValidClientReadParamFromKnownSourceReturnsTrue)
{
	auto config = MakeValidCoreTelematicsConfig();
	bool result = ::middleware::parameter::Domain::Load(config);
	EXPECT_TRUE(result);
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, LoadClientReadParamNotInAnySourceReturnsFalse)
{
	::middleware::parameter::ParameterService config{};
	config.server_identity = "CoreTelematics";
	config.client_read_parameters.push_back({"NonExistentParameter"});
	bool result = ::middleware::parameter::Domain::Load(config);
	EXPECT_FALSE(result);
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, LoadClientWriteParamNotInAnySourceReturnsFalse)
{
	::middleware::parameter::ParameterService config{};
	config.server_identity = "CoreTelematics";
	config.client_write_parameters.push_back({"NonExistentParameter"});
	bool result = ::middleware::parameter::Domain::Load(config);
	EXPECT_FALSE(result);
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, LoadSubscribedParamNotInAnySourceReturnsFalse)
{
	::middleware::parameter::ParameterService config{};
	config.server_identity = "CoreTelematics";
	config.subscribed_parameters.push_back({"NonExistentParameter"});
	bool result = ::middleware::parameter::Domain::Load(config);
	EXPECT_FALSE(result);
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, LoadNoPublishedOrClientParamsReturnsTrue)
{
	::middleware::parameter::ParameterService config{};
	config.server_identity = "Display";
	bool result            = ::middleware::parameter::Domain::Load(config);
	EXPECT_TRUE(result);
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, LoadServerWriteParamNotOwnedReturnsFalse)
{
	::middleware::parameter::ParameterService config{};
	config.server_identity = "Display";
	config.server_write_parameters.push_back({"EngineSpeed"});  // Owned by DataLinkEngine
	bool result = ::middleware::parameter::Domain::Load(config);
	EXPECT_FALSE(result);
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, LoadServerReadParamNotOwnedReturnsFalse)
{
	::middleware::parameter::ParameterService config{};
	config.server_identity = "Display";
	config.server_read_parameters.push_back({"GPSData"});  // Owned by CoreTelematics
	bool result = ::middleware::parameter::Domain::Load(config);
	EXPECT_FALSE(result);
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, LoadValidServerWriteParamReturnsTrue)
{
	::middleware::parameter::ParameterService config{};
	config.server_identity = "Display";
	config.server_write_parameters.push_back({"Brightness"});
	bool result = ::middleware::parameter::Domain::Load(config);
	EXPECT_TRUE(result);
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, LoadValidServerReadParamReturnsTrue)
{
	::middleware::parameter::ParameterService config{};
	config.server_identity = "Display";
	config.server_read_parameters.push_back({"Language"});
	bool result = ::middleware::parameter::Domain::Load(config);
	EXPECT_TRUE(result);
}

// ---- GetClientConnections ----

TEST_F(
    ParameterEndpointConfigurationUtilitiesTest,
    GetClientConnectionsWithConfiguredClientReadParamsReturnsConnections
)
{
	auto config = MakeValidCoreTelematicsConfig();
	::middleware::parameter::Domain::Load(config);

	auto connections = ::middleware::parameter::Domain::GetClientConnections(config);
	EXPECT_FALSE(connections.empty());
	EXPECT_THAT(connections, Contains(Key("DataLinkEngine")));
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, GetClientConnectionsDoesNotContainSelfAsSource)
{
	::middleware::parameter::ParameterService config{};
	config.server_identity = "DataLinkEngine";
	config.client_read_parameters.push_back({"GPSData"});
	::middleware::parameter::Domain::Load(config);

	auto connections = ::middleware::parameter::Domain::GetClientConnections(config);
	EXPECT_THAT(connections, Not(Contains(Key("DataLinkEngine"))));
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, GetClientConnectionsNoClientParamsDefaultsToDataLinkEngine)
{
	::middleware::parameter::ParameterService config{};
	config.server_identity = "CoreTelematics";
	::middleware::parameter::Domain::Load(config);

	auto connections = ::middleware::parameter::Domain::GetClientConnections(config);
	EXPECT_FALSE(connections.empty());
	EXPECT_THAT(connections, Contains(Key("DataLinkEngine")));
}

TEST_F(
    ParameterEndpointConfigurationUtilitiesTest,
    GetClientConnectionsUnknownReadAndWriteParamsDefaultToDataLinkEngine
)
{
	::middleware::parameter::ParameterService config{};
	config.server_identity = "CoreTelematics";
	config.client_read_parameters.push_back({"UnknownReadParam"});
	config.client_write_parameters.push_back({"UnknownWriteParam"});
	::middleware::parameter::Domain::Load(config);

	auto connections = ::middleware::parameter::Domain::GetClientConnections(config);
	EXPECT_THAT(connections, Contains(Key("DataLinkEngine")));
	EXPECT_THAT(
	    connections.at("DataLinkEngine"),
	    Contains(Field(&::middleware::parameter::Information::parameter_name, Eq("UnknownReadParam")))
	);
	EXPECT_THAT(
	    connections.at("DataLinkEngine"),
	    Contains(Field(&::middleware::parameter::Information::parameter_name, Eq("UnknownWriteParam")))
	);
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, GetClientConnectionsClientWriteParamsResolvedReturnsCorrectSource)
{
	::middleware::parameter::ParameterService config{};
	config.server_identity = "Display";
	config.client_write_parameters.push_back({"GPSData"});
	::middleware::parameter::Domain::Load(config);

	auto connections = ::middleware::parameter::Domain::GetClientConnections(config);
	EXPECT_THAT(connections, Contains(Key("CoreTelematics")));
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, GetClientConnectionsMultipleSourcesConfiguredReturnsAllSources)
{
	::middleware::parameter::ParameterService config{};
	config.server_identity = "Display";
	config.client_read_parameters.push_back({"EngineSpeed"});      // DataLinkEngine
	config.client_read_parameters.push_back({"GPSData"});          // CoreTelematics
	config.client_read_parameters.push_back({"UserProfileName"});  // SomeBlueKey
	::middleware::parameter::Domain::Load(config);

	auto connections = ::middleware::parameter::Domain::GetClientConnections(config);
	EXPECT_THAT(connections, Contains(Key("DataLinkEngine")));
	EXPECT_THAT(connections, Contains(Key("CoreTelematics")));
	EXPECT_THAT(connections, Contains(Key("SomeBlueKey")));
}

// ---- n_ParameterDataTypeAsString / n_ParameterDataTypeAsEnum ----

TEST_F(ParameterEndpointConfigurationUtilitiesTest, DataTypeAsStringAllEnumsHaveStringMapping)
{
	EXPECT_THAT(
	    ::middleware::parameter::n_ParameterDataTypeAsString.at(::middleware::parameter::DATA_TYPE_UNSPECIFIED),
	    Eq("unspecified")
	);
	EXPECT_THAT(
	    ::middleware::parameter::n_ParameterDataTypeAsString.at(::middleware::parameter::DATA_TYPE_DOUBLE), Eq("double")
	);
	EXPECT_THAT(
	    ::middleware::parameter::n_ParameterDataTypeAsString.at(::middleware::parameter::DATA_TYPE_STRING), Eq("string")
	);
	EXPECT_THAT(
	    ::middleware::parameter::n_ParameterDataTypeAsString.at(::middleware::parameter::DATA_TYPE_BINARY), Eq("binary")
	);
	EXPECT_THAT(
	    ::middleware::parameter::n_ParameterDataTypeAsString.at(::middleware::parameter::DATA_TYPE_BOOL), Eq("bool")
	);
	EXPECT_THAT(
	    ::middleware::parameter::n_ParameterDataTypeAsString.at(::middleware::parameter::DATA_TYPE_UINT64),
	    Eq("unsigned")
	);
	EXPECT_THAT(
	    ::middleware::parameter::n_ParameterDataTypeAsString.at(::middleware::parameter::DATA_TYPE_MAX), Eq("max")
	);
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, DataTypeAsEnumAllStringsHaveEnumMapping)
{
	EXPECT_THAT(
	    ::middleware::parameter::n_ParameterDataTypeAsEnum.at("unspecified"),
	    Eq(::middleware::parameter::DATA_TYPE_UNSPECIFIED)
	);
	EXPECT_THAT(
	    ::middleware::parameter::n_ParameterDataTypeAsEnum.at("double"), Eq(::middleware::parameter::DATA_TYPE_DOUBLE)
	);
	EXPECT_THAT(
	    ::middleware::parameter::n_ParameterDataTypeAsEnum.at("string"), Eq(::middleware::parameter::DATA_TYPE_STRING)
	);
	EXPECT_THAT(
	    ::middleware::parameter::n_ParameterDataTypeAsEnum.at("binary"), Eq(::middleware::parameter::DATA_TYPE_BINARY)
	);
	EXPECT_THAT(
	    ::middleware::parameter::n_ParameterDataTypeAsEnum.at("bool"), Eq(::middleware::parameter::DATA_TYPE_BOOL)
	);
	EXPECT_THAT(
	    ::middleware::parameter::n_ParameterDataTypeAsEnum.at("unsigned"), Eq(::middleware::parameter::DATA_TYPE_UINT64)
	);
	EXPECT_THAT(
	    ::middleware::parameter::n_ParameterDataTypeAsEnum.at("max"), Eq(::middleware::parameter::DATA_TYPE_MAX)
	);
}

TEST_F(ParameterEndpointConfigurationUtilitiesTest, DataTypeMapsAreInverseOfEachOther)
{
	for(auto const& [enumVal, strVal] : ::middleware::parameter::n_ParameterDataTypeAsString)
	{
		auto it = ::middleware::parameter::n_ParameterDataTypeAsEnum.find(strVal);
		ASSERT_NE(it, std::cend(::middleware::parameter::n_ParameterDataTypeAsEnum));
		EXPECT_THAT(it->second, Eq(enumVal));
	}
}
