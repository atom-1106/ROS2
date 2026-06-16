// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: SpyApp.h
// Description: Middleware Data Trace Callbacks

#ifndef SpyApp_H
#define SpyApp_H

#include "LogUtility.h"
#include "MiddlewareSpy.h"

#include <middleware_diagnostics.pb.h>
#include <middleware_parameter.pb.h>

#include <iostream>
#include <sstream>

class SpyApp : MiddlewareSpy
{
	void subscriberDiscovered(MiddlewareSpyParticipantData const& participant) override
	{
		std::stringstream ss{};
		ss << std::endl;
		ss << "New / Modified Subscriber:" << std::endl;
		ss << "\tGUID Prefix: " << participant.guid->guidPrefix << std::endl;
		ss << "\tGUID ID:     " << participant.guid->entityId << std::endl;
		ss << "\tTopic:       " << participant.topic << std::endl;
		std::cout << ss.str() << std::endl;
	};
	void publisherDiscovered(MiddlewareSpyParticipantData const& participant) override
	{
		std::stringstream ss{};
		ss << std::endl;
		ss << "New / Modified Publisher:" << std::endl;
		ss << "\tGUID Prefix: " << participant.guid->guidPrefix << std::endl;
		ss << "\tGUID ID:     " << participant.guid->entityId << std::endl;
		ss << "\tTopic:       " << participant.topic << std::endl;
		std::cout << ss.str() << std::endl;
	};
	void subscriberRemoved(MiddlewareSpyParticipantData const& participant) override
	{
		std::stringstream ss{};
		ss << std::endl;
		ss << "Removed Subscriber:" << std::endl;
		ss << "\tGUID Prefix: " << participant.guid->guidPrefix << std::endl;
		ss << "\tGUID ID:     " << participant.guid->entityId << std::endl;
		ss << "\tTopic:       " << participant.topic << std::endl;
		std::cout << ss.str() << std::endl;
	};
	void publisherRemoved(MiddlewareSpyParticipantData const& participant) override
	{
		std::stringstream ss{};
		ss << std::endl;
		ss << "Removed Publisher:" << std::endl;
		ss << "\tGUID Prefix: " << participant.guid->guidPrefix << std::endl;
		ss << "\tGUID ID:     " << participant.guid->entityId << std::endl;
		ss << "\tTopic:       " << participant.topic << std::endl;
		std::cout << ss.str() << std::endl;
	};
	void topicSubscriberData(std::string const& topic, std::vector<uint8_t> const& data) override
	{
		if(topic.find("Diagnostics") == std::string::npos)
		{
			::cat::middleware::parameter::Data parameterData{};
			parameterData.ParseFromString(std::string(std::cbegin(data), std::cend(data)));
			utility::LogParameterData(topic, parameterData);
		}
		else
		{
			::cat::middleware::diagnostics::Codes codes{};
			codes.ParseFromString(std::string(std::cbegin(data), std::cend(data)));
			utility::LogDiagnosticCode(topic, codes);
		}
	}
};
#endif  // SpyApp_H
