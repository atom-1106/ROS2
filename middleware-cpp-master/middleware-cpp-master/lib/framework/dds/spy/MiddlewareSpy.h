// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: MiddlewareSpy.h
// Description: Middleware Spy datatypes, helpers, and main class

#ifndef MiddlewareSpy_H
#define MiddlewareSpy_H

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/rtps/builtin/data/ParticipantBuiltinTopicData.hpp>
#include <fastdds/rtps/builtin/data/PublicationBuiltinTopicData.hpp>
#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/reader/ReaderDiscoveryStatus.hpp>
#include <fastdds/rtps/writer/WriterDiscoveryStatus.hpp>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include "MiddlewareSpy.h"

namespace middleware
{
class IMiddlewareParticipant;
class IMiddlewareReader;
}  // namespace middleware

struct MiddlewareSpyParticipantData
{
	::eprosima::fastdds::rtps::GUID_t const* const guid;
	std::string const& topic;
};

struct ByParticipantGUID_tHash
{
	std::size_t operator()(::eprosima::fastdds::rtps::GUID_t const& guid) const
	{
		std::stringstream ss{};
		ss << guid.guidPrefix;
		// GUID IDs don't appear to be shared between Participants and Readers/Writers
		// so don't combine ID hash, just return prefix hash
		return std::hash<std::string>{}(ss.str());
	}
};
struct GUID_tHash
{
	std::size_t operator()(::eprosima::fastdds::rtps::GUID_t const& guid) const
	{
		std::stringstream ss{};
		ss << guid.guidPrefix;
		std::size_t h1 = std::hash<std::string>{}(ss.str());
		ss.str(std::string());
		ss << guid.entityId;
		std::size_t h2 = std::hash<std::string>{}(ss.str());
		return h1 ^ (h2 << 1);
	}
};
struct ByParticipantGUID_tEqual
{
	bool operator()(::eprosima::fastdds::rtps::GUID_t const& a, ::eprosima::fastdds::rtps::GUID_t const& b) const
	{
		return a.guidPrefix == b.guidPrefix;
	}
};
// Fast-DDS implements == that compares GUID IDs

class MiddlewareSpy : public ::eprosima::fastdds::dds::DomainParticipantListener
{
public:
	MiddlewareSpy();
	~MiddlewareSpy();

	// User-defined callbacks
	virtual bool tapTopic(std::string topic)
	{
		return true;
	};
	virtual void subscriberDiscovered(MiddlewareSpyParticipantData const& participant) {};
	virtual void publisherDiscovered(MiddlewareSpyParticipantData const& participant) {};
	virtual void subscriberRemoved(MiddlewareSpyParticipantData const& participant) {};
	virtual void publisherRemoved(MiddlewareSpyParticipantData const& participant) {};
	virtual void topicSubscriberData(std::string const& topic, std::vector<uint8_t> const& data) {};

	// DomainParticipantListener
	void on_participant_discovery(
	    ::eprosima::fastdds::dds::DomainParticipant* participant,
	    ::eprosima::fastdds::rtps::ParticipantDiscoveryStatus status,
	    ::eprosima::fastdds::rtps::ParticipantBuiltinTopicData const& info,
	    bool& should_be_ignored
	) override final;
	void on_data_reader_discovery(
	    ::eprosima::fastdds::dds::DomainParticipant* participant,
	    ::eprosima::fastdds::rtps::ReaderDiscoveryStatus status,
	    ::eprosima::fastdds::rtps::SubscriptionBuiltinTopicData const& info,
	    bool& should_be_ignored
	) override final;
	void on_data_writer_discovery(
	    ::eprosima::fastdds::dds::DomainParticipant* participant,
	    ::eprosima::fastdds::rtps::WriterDiscoveryStatus status,
	    ::eprosima::fastdds::rtps::PublicationBuiltinTopicData const& info,
	    bool& should_be_ignored
	) override final;

private:
	// use unique participant qos name to filter out self
	std::string const m_qosName = "Spy_subscriber";

	void AddWiretap(std::string const& topic);
	void RemoveWiretap(std::string const& topic);

	// Fast-DDS / Middleware storage
	::eprosima::fastdds::dds::DomainParticipant* m_pDiscoveryParticipant;
	::eprosima::fastdds::dds::TypeSupport m_topicSupportType;
	::eprosima::fastdds::dds::Subscriber* m_pDiscoverySubscriber;
	std::unique_ptr<::middleware::IMiddlewareParticipant> m_spMiddlewareParticipant;
	std::unordered_map<std::string, std::unique_ptr<::middleware::IMiddlewareReader>> m_middlewareSubscribers;
	// Data
	// ___DiscoveryStatus.info ref gets deleted after the callback so store it directly
	// (___BuiltinTopicData instead of ___DiscoveryStatus - the latter doesn't have anything else important anyway)
	std::unordered_map<
	    ::eprosima::fastdds::rtps::GUID_t,
	    ::eprosima::fastdds::rtps::ParticipantBuiltinTopicData,
	    ByParticipantGUID_tHash,
	    ByParticipantGUID_tEqual>
	    m_participantData;
	std::unordered_map<
	    ::eprosima::fastdds::rtps::GUID_t,
	    ::eprosima::fastdds::rtps::SubscriptionBuiltinTopicData,
	    ByParticipantGUID_tHash,
	    ByParticipantGUID_tEqual>
	    m_subscriberData;
	std::unordered_map<
	    ::eprosima::fastdds::rtps::GUID_t,
	    ::eprosima::fastdds::rtps::PublicationBuiltinTopicData,
	    ByParticipantGUID_tHash,
	    ByParticipantGUID_tEqual>
	    m_publisherData;

	// Associations
	std::multimap<std::string, ::eprosima::fastdds::rtps::GUID_t> m_topicSubscribers;
	std::multimap<std::string, ::eprosima::fastdds::rtps::GUID_t> m_topicPublishers;
	std::multiset<std::string> m_tappedTopics;
};

#endif  // MiddlewareSpy_H
